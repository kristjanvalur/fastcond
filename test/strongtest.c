/* Copyright (c) 2017-2025 Kristján Valur Jónsson */

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
 * Similar to the qtest, this version has only one condition variable, and
 * a single producer and server.  It tests how a `strong` condition variable wakes up
 * at least one of the threads already waiting, i.e. that the wakeup cannot be
 * stolen by the other thread.
 */

typedef struct _queue {
    pthread_mutex_t mutex;
    int n_data; /* current size of queue */
    pthread_cond_t cond;
    int max_queue;               /* how much fits in the queue */
    int max_send;                /* how many packets to send? (termination test) */
    int n_sent;                  /* total amount sent */
    struct timespec *timestamps; /* track enqueue times for latency measurement */
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
        while (q->n_sent < q->max_send && q->n_data >= q->max_queue)
            /* queue is active and full, wake wait */
            pthread_cond_wait(&q->cond, &q->mutex);
        if (q->n_sent < q->max_send) {
            /* Enqueue: record timestamp */
            if (q->timestamps && q->n_data < q->max_queue) {
                clock_gettime(CLOCK_MONOTONIC, &q->timestamps[q->n_data]);
            }
            q->n_data++;
            q->n_sent++;
            n_sent++;
            have_data = 0;
            pthread_cond_signal(&q->cond); /* wake up other thread */
            /* wake up rest of senders and receivers if we trigger end condition */
            if (q->n_sent == q->max_send) {
                pthread_cond_broadcast(&q->cond);
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
    float sum_time = 0.0, sum_time2 = 0.0;
    float min_time = 1e9, max_time = 0.0;
    struct timespec now, enqueue_time;
    pthread_mutex_lock(&q->mutex);
    while (q->n_sent < q->max_send || q->n_data) {
        if (have_data) {
            /* simulate getting rid of the data */
            pthread_mutex_unlock(&q->mutex);

            /* Compute latency if we have timestamp data */
            if (q->timestamps) {
                now.tv_sec -= enqueue_time.tv_sec;
                now.tv_nsec -= enqueue_time.tv_nsec;
                if (now.tv_nsec < 0) {
                    now.tv_sec--;
                    now.tv_nsec += 1000000000;
                }
                float time = (float) now.tv_sec + (float) now.tv_nsec * 1e-9f;
                sum_time += time;
                sum_time2 += time * time;
                if (time < min_time)
                    min_time = time;
                if (time > max_time)
                    max_time = time;
            }

            sched_yield();
            pthread_mutex_lock(&q->mutex);
            have_data = 0;
        }
        while (q->n_sent < q->max_send && !q->n_data)
            /* queue is active and empty */
            pthread_cond_wait(&q->cond, &q->mutex);
        if (q->n_data) {
            /* Dequeue: capture current time and get enqueue timestamp */
            clock_gettime(CLOCK_MONOTONIC, &now);
            if (q->timestamps && q->n_data > 0) {
                enqueue_time = q->timestamps[q->n_data - 1];
            }
            q->n_data--;
            n_got++;
            have_data = 1;
            pthread_cond_signal(&q->cond); /* wake up sender */
        }
    }
    pthread_mutex_unlock(&q->mutex);

    /* Compute and print stats */
    if (q->timestamps) {
        float avg = 0.0f, variance = 0.0f, stdev = 0.0f;
        if (n_got > 0) {
            avg = sum_time / (float) n_got;
            if (n_got > 1) {
                variance =
                    (sum_time2 - (sum_time * sum_time) / (float) n_got) / (float) (n_got - 1);
                stdev = sqrtf(variance);
            }
        } else {
            min_time = 0.0f;
            max_time = 0.0f;
        }
        printf("receiver %d got %d latency avg %e stdev %e min %e max %e\n", args->id, n_got, avg,
               stdev, min_time, max_time);
    } else {
        printf("receiver %d got %d\n", args->id, n_got);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int n_data = 1000;
    int n_senders = 1;
    int n_receivers = 1;
    int max_queue = 1;
    queue_t q;
    int i;
    args_t *senders, *receivers;
    void *retval;
    struct timespec start_time, end_time;
    double elapsed_sec;

    if (argc > 1)
        n_data = atoi(argv[1]);
    if (argc > 2)
        max_queue = atoi(argv[2]);

    q.n_data = q.n_sent = 0;
    q.max_queue = max_queue;
    q.max_send = n_data;
    q.timestamps = (struct timespec *) malloc(max_queue * sizeof(struct timespec));
    pthread_mutex_init(&q.mutex, NULL);
    pthread_cond_init(&q.cond, NULL);

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
    for (i = 0; i < n_receivers; i++)
        pthread_join(receivers[i].pid, &retval);
    for (i = 0; i < n_senders; i++)
        pthread_join(senders[i].pid, &retval);

    /* End timing */
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    /* Calculate elapsed time */
    elapsed_sec =
        (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) * 1e-9;

    /* Print overall statistics */
    printf("=== Overall Statistics ===\n");
    printf("Total items: %d\n", n_data);
    printf("Threads: %d senders, %d receivers\n", n_senders, n_receivers);
    printf("Queue size: %d\n", max_queue);
    printf("Total time: %.6f seconds\n", elapsed_sec);
    printf("Throughput: %.2f items/sec\n", n_data / elapsed_sec);
    printf("==========================\n");

    /* cleanup */
    pthread_cond_destroy(&q.cond);
    pthread_mutex_destroy(&q.mutex);
    free(q.timestamps);
    free(receivers);
    free(senders);

    return 0;
}