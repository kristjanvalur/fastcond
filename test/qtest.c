
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "fastcond.h"
#ifdef TEST_WCOND
#define FASTCOND_PATCH_WCOND
#endif
#ifdef TEST_COND
#define FASTCOND_PATCH_COND
#endif
#include "fastcond_patch.h"

/*
 * This code tests parallelism by implementing a producer-consumer system
 * and push stuff through the queue
 */

typedef struct _queue {
    pthread_mutex_t mutex;

    struct timespec *queue; /* queue contains times when it was put in */
    int s_queue;            /* max size of queue */
    int i_queue;
    int n_queue; /* current size of queue */

    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    int max_send; /* how many packets to send? (termination test) */
    int n_sent;   /* total amount sent */
} queue_t;

typedef struct _args {
    queue_t *queue;
    int id;
    pthread_t pid;
} args_t;

void *sender(void *arg)
{
    args_t *args = (args_t *) arg;
    queue_t *q = args->queue;
    int n_sent = 0;
    int have_data = 0;
    pthread_mutex_lock(&q->mutex);
    while (q->n_sent < q->max_send) {
        if (!have_data) {
            /* simulate getting of the data */
            pthread_mutex_unlock(&q->mutex);
            sched_yield();
            pthread_mutex_lock(&q->mutex);
            have_data = 1;
        }
        while (q->n_sent < q->max_send && q->n_queue >= q->s_queue)
            /* queue is active and full */
            pthread_cond_wait(&q->not_full, &q->mutex);
        if (q->n_sent < q->max_send && q->n_queue < q->s_queue) {
            /* put stuff on queue */
            int i = (q->i_queue + q->n_queue) % q->s_queue;
            clock_gettime(CLOCK_MONOTONIC, &q->queue[i]);
            q->n_queue++;

            q->n_sent++;
            n_sent++;
            have_data = 0;
            pthread_cond_signal(&q->not_empty);
            /* wake up rest of senders and receivers if we trigger end condition */
            if (q->n_sent == q->max_send) {
                pthread_cond_broadcast(&q->not_full);
                pthread_cond_broadcast(&q->not_empty);
            }
        }
    }
    pthread_mutex_unlock(&q->mutex);
    printf("sender %d sent %d\n", args->id, n_sent);
    return NULL;
}

void *receiver(void *arg)
{
    args_t *args = (args_t *) arg;
    queue_t *q = args->queue;
    int n_got = 0;
    int have_data = 0;
    float time, sum_time = 0.0, sum_time2 = 0.0; /* stats */
    float min_time = 1e9, max_time = 0.0; /* min/max tracking */
    struct timespec data, now;
    pthread_mutex_lock(&q->mutex);
    while (q->n_sent < q->max_send || q->n_queue) {
        if (have_data) {
            /* simulate getting rid of the data */
            pthread_mutex_unlock(&q->mutex);
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
            sched_yield();
            pthread_mutex_lock(&q->mutex);
            have_data = 0;
        }
        while (q->n_sent < q->max_send && !q->n_queue)
            /* queue is active and empty */
            pthread_cond_wait(&q->not_empty, &q->mutex);
        if (q->n_queue) {
            /* remove from queue */
            clock_gettime(CLOCK_MONOTONIC, &now);
            data = q->queue[q->i_queue];
            q->i_queue = (q->i_queue + 1) % q->s_queue;
            q->n_queue--;

            n_got++;
            have_data = 1;
            pthread_cond_signal(&q->not_full);
        }
    }
    pthread_mutex_unlock(&q->mutex);
    /* compute stats */
    {
        float avg = 0.0f, variance = 0.0f, stdev = 0.0f;
        if (n_got > 0) {
            avg = sum_time / (float) n_got;
            /* Correct variance formula: Var(X) = E[X²] - E[X]² */
            if (n_got > 1) {
                variance = (sum_time2 - (sum_time * sum_time) / (float) n_got) / (float) (n_got - 1);
                stdev = sqrtf(variance);
            }
        } else {
            /* No data collected - set min/max to 0 */
            min_time = 0.0f;
            max_time = 0.0f;
        }

        printf("receiver %d got %d latency avg %e stdev %e min %e max %e\n", 
               args->id, n_got, avg, stdev, min_time, max_time);
    }
    return NULL;
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
    struct timespec start_time, end_time;
    double elapsed_sec;

    /* Unbuffer stdout to ensure output appears immediately */
    setbuf(stdout, NULL);
    printf("qtest starting (argc=%d)\n", argc);
    fflush(stdout);

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
    q.queue = (struct timespec *) malloc(s_queue * sizeof(struct timespec));

    q.n_sent = 0;
    q.max_send = n_data;
    pthread_mutex_init(&q.mutex, NULL);
    pthread_cond_init(&q.not_empty, NULL);
    pthread_cond_init(&q.not_full, NULL);

    /* Start timing */
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    /* create receivers */
    receivers = (args_t *) malloc(n_receivers * sizeof(args_t));
    for (i = 0; i < n_receivers; i++) {
        receivers[i].id = i;
        receivers[i].queue = &q;
        pthread_create(&receivers[i].pid, NULL, &receiver, (void *) &receivers[i]);
    }

    /* create senders */
    senders = (args_t *) malloc(n_senders * sizeof(args_t));
    for (i = 0; i < n_senders; i++) {
        senders[i].id = i;
        senders[i].queue = &q;
        pthread_create(&senders[i].pid, NULL, &sender, (void *) &senders[i]);
    }

    /* wait for senders and receivers to end */
    for (i = 0; i < n_receivers; i++) {
        pthread_join(receivers[i].pid, &retval);
    }
    for (i = 0; i < n_senders; i++) {
        pthread_join(senders[i].pid, &retval);
    }

    /* End timing */
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    /* Calculate elapsed time */
    elapsed_sec = (end_time.tv_sec - start_time.tv_sec) + 
                  (end_time.tv_nsec - start_time.tv_nsec) * 1e-9;
    
    /* Print overall statistics */
    printf("=== Overall Statistics ===\n");
    printf("Total items: %d\n", n_data);
    printf("Threads: %d senders, %d receivers\n", n_senders, n_receivers);
    printf("Queue size: %d\n", s_queue);
    printf("Total time: %.6f seconds\n", elapsed_sec);
    printf("Throughput: %.2f items/sec\n", n_data / elapsed_sec);
    printf("==========================\n");
    
    printf("All threads completed, cleaning up\n");
    fflush(stdout);

    /* cleanup */
    pthread_cond_destroy(&q.not_empty);
    pthread_cond_destroy(&q.not_full);
    pthread_mutex_destroy(&q.mutex);
    free(q.queue);
    free(receivers);
    free(senders);

    return 0;
}