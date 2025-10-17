
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
    printf("sender %d starting\n", args->id);
    fflush(stdout);
    pthread_mutex_lock(&q->mutex);
    while (q->n_sent < q->max_send) {
        if (!have_data) {
            /* simulate getting of the data */
            pthread_mutex_unlock(&q->mutex);
            sched_yield();
            pthread_mutex_lock(&q->mutex);
            have_data = 1;
        }
        while (q->n_sent < q->max_send && q->n_queue >= q->s_queue) {
            /* queue is active and full */
            printf("sender %d waiting (queue full: %d/%d, sent %d/%d)\n", args->id,
                   q->n_queue, q->s_queue, q->n_sent, q->max_send);
            fflush(stdout);
            pthread_cond_wait(&q->not_full, &q->mutex);
            printf("sender %d woke up\n", args->id);
            fflush(stdout);
        }
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
    struct timespec data, now;
    printf("receiver %d starting\n", args->id);
    fflush(stdout);
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
            sched_yield();
            pthread_mutex_lock(&q->mutex);
            have_data = 0;
        }
        while (q->n_sent < q->max_send && !q->n_queue) {
            /* queue is active and empty */
            printf("receiver %d waiting (queue empty: %d/%d, sent %d/%d)\n", args->id,
                   q->n_queue, q->s_queue, q->n_sent, q->max_send);
            fflush(stdout);
            pthread_cond_wait(&q->not_empty, &q->mutex);
            printf("receiver %d woke up\n", args->id);
            fflush(stdout);
        }
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
        float avg = 0.0f, sigma = 0.0f;
        if (n_got) {
            avg = sum_time / (float) n_got;
            sigma = sqrtf((float) n_got * sum_time - sum_time2) / (float) n_got;
        }

        printf("receiver %d got %d latency avg %e stdev %e\n", args->id, n_got, avg, sigma);
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
    printf("Initializing mutex and condition variables\n");
    fflush(stdout);
    pthread_mutex_init(&q.mutex, NULL);
    pthread_cond_init(&q.not_empty, NULL);
    pthread_cond_init(&q.not_full, NULL);
    printf("Creating threads: %d senders, %d receivers, queue size %d, data count %d\n",
           n_senders, n_receivers, s_queue, n_data);
    fflush(stdout);

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
    printf("Waiting for threads to complete\n");
    fflush(stdout);
    for (i = 0; i < n_receivers; i++) {
        printf("Joining receiver %d\n", i);
        fflush(stdout);
        pthread_join(receivers[i].pid, &retval);
    }
    for (i = 0; i < n_senders; i++) {
        printf("Joining sender %d\n", i);
        fflush(stdout);
        pthread_join(senders[i].pid, &retval);
    }
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