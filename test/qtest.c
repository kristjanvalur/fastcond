/* Copyright (c) 2017-2025 Kristján Valur Jónsson */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fastcond.h"
#include "native_primitives.h"
#include "test_portability.h"

/*
 * This code tests parallelism by implementing a producer-consumer system
 * and push stuff through the queue.
 *
 * Compile-time options:
 *   (none)      - Use native condition variables (pthread_cond_t or CONDITION_VARIABLE)
 *   -DTEST_COND - Use fastcond strong condition variable
 *   -DTEST_WCOND - Use fastcond weak condition variable
 *
 * Environment variables:
 *   FASTCOND_CSV_OUTPUT - If set, output results in CSV format to specified file
 *   FASTCOND_JSON_OUTPUT - If set to "1", output results as JSON to stdout
 *   FASTCOND_PLATFORM - Platform name for CSV output (e.g., "linux", "macos", "windows")
 *   FASTCOND_OS_VERSION - OS version for CSV output (e.g., "ubuntu-latest")
 */

#if defined(TEST_WCOND)
typedef fastcond_wcond_t cond_t;
#define COND_INIT(c) fastcond_wcond_init(&(c), NULL)
#define COND_DESTROY(c) fastcond_wcond_fini(&(c))
#define COND_WAIT(c, m) fastcond_wcond_wait(&(c), &(m))
#define COND_SIGNAL(c) fastcond_wcond_signal(&(c))
#define COND_BROADCAST(c) fastcond_wcond_broadcast(&(c))
#elif defined(TEST_COND)
typedef fastcond_cond_t cond_t;
#define COND_INIT(c) fastcond_cond_init(&(c), NULL)
#define COND_DESTROY(c) fastcond_cond_fini(&(c))
#define COND_WAIT(c, m) fastcond_cond_wait(&(c), &(m))
#define COND_SIGNAL(c) fastcond_cond_signal(&(c))
#define COND_BROADCAST(c) fastcond_cond_broadcast(&(c))
#else
typedef native_cond_t cond_t;
#define COND_INIT(c) NATIVE_COND_INIT(&(c))
#define COND_DESTROY(c) NATIVE_COND_DESTROY(&(c))
#define COND_WAIT(c, m) NATIVE_COND_WAIT(&(c), &(m))
#define COND_SIGNAL(c) NATIVE_COND_SIGNAL(&(c))
#define COND_BROADCAST(c) NATIVE_COND_BROADCAST(&(c))
#endif

/*
 * This code tests parallelism by implementing a producer-consumer system
 * and push stuff through the queue
 */

typedef struct _queue {
    native_mutex_t mutex;

    test_timespec_t *queue; /* queue contains times when it was put in */
    int s_queue;            /* max size of queue */
    int i_queue;
    int n_queue; /* current size of queue */

    cond_t not_empty;
    cond_t not_full;
    int max_send; /* how many packets to send? (termination test) */
    int n_sent;   /* total amount sent */
} queue_t;

typedef struct _args {
    queue_t *queue;
    int id;
    test_thread_t pid;
    /* Stats populated by receiver threads */
    int n_got;
    int n_waits;
    int n_successful_waits;
    float latency_avg;
    float latency_stdev;
    float latency_min;
    float latency_max;
} args_t;

TEST_THREAD_FUNC_RETURN sender(void *arg)
{
    args_t *args = (args_t *) arg;
    queue_t *q = args->queue;
    int n_sent = 0;
    int have_data = 0;
    NATIVE_MUTEX_LOCK(&q->mutex);
    while (q->n_sent < q->max_send) {
        if (!have_data) {
            /* simulate getting of the data */
            NATIVE_MUTEX_UNLOCK(&q->mutex);
            test_sched_yield();
            NATIVE_MUTEX_LOCK(&q->mutex);
            have_data = 1;
        }
        while (q->n_sent < q->max_send && q->n_queue >= q->s_queue)
            /* queue is active and full */
            COND_WAIT(q->not_full, q->mutex);
        if (q->n_sent < q->max_send && q->n_queue < q->s_queue) {
            /* put stuff on queue */
            int i = (q->i_queue + q->n_queue) % q->s_queue;
            test_clock_gettime(&q->queue[i]);
            q->n_queue++;

            q->n_sent++;
            n_sent++;
            have_data = 0;
            COND_SIGNAL(q->not_empty);
            /* wake up rest of senders and receivers if we trigger end condition */
            if (q->n_sent == q->max_send) {
                COND_BROADCAST(q->not_full);
                COND_BROADCAST(q->not_empty);
            }
        }
    }
    NATIVE_MUTEX_UNLOCK(&q->mutex);
    /* Only print if not in JSON mode */
    const char *json_output = getenv("FASTCOND_JSON_OUTPUT");
    if (!json_output || strcmp(json_output, "1") != 0) {
        printf("sender %d sent %d\n", args->id, n_sent);
    }
    TEST_THREAD_RETURN;
}

TEST_THREAD_FUNC_RETURN receiver(void *arg)
{
    args_t *args = (args_t *) arg;
    queue_t *q = args->queue;
    int n_got = 0;
    int n_waits = 0;  /* Total COND_WAIT calls */
    int n_successful_waits = 0;  /* Waits that resulted in data being available */
    int have_data = 0;
    float time, sum_time = 0.0, sum_time2 = 0.0; /* stats */
    float min_time = 1e9, max_time = 0.0;        /* min/max tracking */
    test_timespec_t data, now;
    NATIVE_MUTEX_LOCK(&q->mutex);
    while (q->n_sent < q->max_send || q->n_queue) {
        if (have_data) {
            /* simulate getting rid of the data */
            NATIVE_MUTEX_UNLOCK(&q->mutex);
            /* compute the delay */
            now.tv_sec -= data.tv_sec;
            now.tv_nsec -= data.tv_nsec;
            if (now.tv_nsec < 0) { /* rollover */
                now.tv_sec--;
                now.tv_nsec += 1000000000;
            }
            /* compute stats */
            time = (float) now.tv_sec + (float) now.tv_nsec * 1e-9f;
            sum_time += time;
            sum_time2 += time * time;
            if (time < min_time)
                min_time = time;
            if (time > max_time)
                max_time = time;
            test_sched_yield();
            NATIVE_MUTEX_LOCK(&q->mutex);
            have_data = 0;
        }
        while (q->n_sent < q->max_send && !q->n_queue) {
            /* queue is active and empty */
            COND_WAIT(q->not_empty, q->mutex);
            n_waits++;
            /* Check if we got data after waking up */
            if (q->n_queue > 0) {
                n_successful_waits++;
            }
        }
        if (q->n_queue) {
            /* remove from queue */
            test_clock_gettime(&now);
            data = q->queue[q->i_queue];
            q->i_queue = (q->i_queue + 1) % q->s_queue;
            q->n_queue--;

            n_got++;
            have_data = 1;
            COND_SIGNAL(q->not_full);
        }
    }
    NATIVE_MUTEX_UNLOCK(&q->mutex);
    /* compute stats */
    {
        float avg = 0.0f, variance = 0.0f, stdev = 0.0f;
        if (n_got > 0) {
            avg = sum_time / (float) n_got;
            /* Correct variance formula: Var(X) = E[X²] - E[X]² */
            if (n_got > 1) {
                variance =
                    (sum_time2 - (sum_time * sum_time) / (float) n_got) / (float) (n_got - 1);
                stdev = sqrtf(variance);
            }
        } else {
            /* No data collected - set min/max to 0 */
            min_time = 0.0f;
            max_time = 0.0f;
        }

        /* Store stats in args structure */
        args->n_got = n_got;
        args->n_waits = n_waits;
        args->n_successful_waits = n_successful_waits;
        args->latency_avg = avg;
        args->latency_stdev = stdev;
        args->latency_min = min_time;
        args->latency_max = max_time;

        /* Calculate spurious wakeups: waits that didn't result in data */
        int spurious_wakeups = n_waits - n_successful_waits;

        /* Only print if not in JSON mode */
        const char *json_output = getenv("FASTCOND_JSON_OUTPUT");
        if (!json_output || strcmp(json_output, "1") != 0) {
            printf("receiver %d got %d latency avg %e stdev %e min %e max %e spurious %d\n", args->id, n_got, avg,
                   stdev, min_time, max_time, spurious_wakeups);
        }
    }
    TEST_THREAD_RETURN;
}

int main(int argc, char *argv[])
{
    int n_data = 1000;
    int n_senders = 1;
    int n_receivers = 1;
    int s_queue = 10;
    queue_t q;
    int i;
    args_t *senders, *receivers;
    void *retval;
    test_timespec_t start_time, end_time;
    double elapsed_sec;

    /* Unbuffer stdout to ensure output appears immediately */
    setbuf(stdout, NULL);
    
    /* Check if JSON mode early to suppress informational output */
    const char *json_output = getenv("FASTCOND_JSON_OUTPUT");
    int json_mode = (json_output && strcmp(json_output, "1") == 0);
    
    if (!json_mode) {
        printf("qtest starting (argc=%d)\n", argc);
        fflush(stdout);
    }

    if (argc > 1)
        n_data = atoi(argv[1]);
    if (argc > 2)
        n_senders = atoi(argv[2]);
    if (argc > 3)
        s_queue = atoi(argv[3]);
    n_receivers = n_senders;

    /* init the queue */
    q.i_queue = q.n_queue = 0;
    q.s_queue = s_queue;
    q.queue = (test_timespec_t *) malloc(s_queue * sizeof(test_timespec_t));

    q.n_sent = 0;
    q.max_send = n_data;
    NATIVE_MUTEX_INIT(&q.mutex);
    COND_INIT(q.not_empty);
    COND_INIT(q.not_full);

    /* Start timing */
    test_clock_gettime(&start_time);

    /* create receivers */
    receivers = (args_t *) malloc(n_receivers * sizeof(args_t));
    for (i = 0; i < n_receivers; i++) {
        receivers[i].id = i;
        receivers[i].queue = &q;
        test_thread_create(&receivers[i].pid, NULL, &receiver, (void *) &receivers[i]);
    }

    /* create senders */
    senders = (args_t *) malloc(n_senders * sizeof(args_t));
    for (i = 0; i < n_senders; i++) {
        senders[i].id = i;
        senders[i].queue = &q;
        test_thread_create(&senders[i].pid, NULL, &sender, (void *) &senders[i]);
    }

    /* wait for senders and receivers to end */
    for (i = 0; i < n_receivers; i++) {
        test_thread_join(receivers[i].pid, &retval);
    }
    for (i = 0; i < n_senders; i++) {
        test_thread_join(senders[i].pid, &retval);
    }

    /* End timing */
    test_clock_gettime(&end_time);

    /* Calculate elapsed time */
    elapsed_sec = test_timespec_diff(&end_time, &start_time);

    /* Determine variant name */
    const char *variant;
#if defined(TEST_WCOND)
    variant = "fastcond_wcond";
#elif defined(TEST_COND)
    variant = "fastcond_cond";
#else
    variant = "native";
#endif

    /* Check if JSON output is requested and output JSON format */
    if (json_mode) {
        /* Output compact JSON for easy parsing */
        printf("{\"test\":\"qtest\",\"variant\":\"%s\",", variant);
        printf("\"config\":{\"n_data\":%d,\"n_senders\":%d,\"n_receivers\":%d,\"queue_size\":%d},", 
               n_data, n_senders, n_receivers, s_queue);
        printf("\"timing\":{\"elapsed_sec\":%.9f,\"throughput\":%.2f},", 
               elapsed_sec, n_data / elapsed_sec);
        printf("\"per_thread\":[");
        for (i = 0; i < n_receivers; i++) {
            int spurious_wakeups = receivers[i].n_waits - receivers[i].n_successful_waits;
            printf("{\"thread\":%d,\"type\":\"receiver\",\"n_got\":%d,\"n_waits\":%d,\"spurious_wakeups\":%d,"
                   "\"latency_avg\":%.12e,\"latency_stdev\":%.12e,\"latency_min\":%.12e,\"latency_max\":%.12e}%s",
                   i, receivers[i].n_got, receivers[i].n_waits, spurious_wakeups,
                   (double)receivers[i].latency_avg, (double)receivers[i].latency_stdev, 
                   (double)receivers[i].latency_min, (double)receivers[i].latency_max,
                   (i < n_receivers - 1) ? "," : "");
        }
        printf("]}\n");
        return 0;
    }

    /* Check if CSV output is requested */
    const char *csv_file = getenv("FASTCOND_CSV_OUTPUT");
    if (csv_file) {
        const char *platform = getenv("FASTCOND_PLATFORM");
        const char *os_version = getenv("FASTCOND_OS_VERSION");

        /* Default values if env vars not set */
        if (!platform)
            platform = "unknown";
        if (!os_version)
            os_version = "unknown";

        FILE *fp = fopen(csv_file, "a");
        if (fp) {
            /* Calculate aggregate latency stats across all receivers */
            double total_latency_sum = 0.0;
            double total_latency_sq_sum = 0.0;
            int total_samples = 0;
            double global_min = 1e9;
            double global_max = 0.0;

            for (i = 0; i < n_receivers; i++) {
                if (receivers[i]
                        .queue) { /* Receivers store latency data in queue field temporarily */
                    /* Note: In actual implementation, we'd need to preserve per-thread stats */
                    /* For now, just output the overall throughput */
                }
            }

            /* Write CSV row:
             * platform,os_version,test,variant,threads,queue_size,iterations,elapsed_sec,throughput_items_per_sec
             */
            fprintf(fp, "%s,%s,qtest,%s,%d,%d,%d,%.6f,%.2f\n", platform, os_version, variant,
                    n_senders, s_queue, n_data, elapsed_sec, n_data / elapsed_sec);
            fclose(fp);
        }
    }

    /* Print overall statistics (skip in JSON mode) */
    if (!json_mode) {
        printf("=== Overall Statistics ===\n");
        printf("Total items: %d\n", n_data);
        printf("Threads: %d senders, %d receivers\n", n_senders, n_receivers);
        printf("Queue size: %d\n", s_queue);
        printf("Total time: %.6f seconds\n", elapsed_sec);
        printf("Throughput: %.2f items/sec\n", n_data / elapsed_sec);
        printf("==========================\n");

        printf("All threads completed, cleaning up\n");
        fflush(stdout);
    }

    /* Cleanup */
    COND_DESTROY(q.not_empty);
    COND_DESTROY(q.not_full);
    NATIVE_MUTEX_DESTROY(&q.mutex);
    free(q.queue);
    free(receivers);
    free(senders);

    return 0;
}