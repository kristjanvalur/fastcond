/* Copyright (c) 2017-2025 Kristján Valur Jónsson */

// Fairness mechanism control - matches definition in gil.c
#ifndef FASTCOND_GIL_DISABLE_FAIRNESS
#define FASTCOND_GIL_DISABLE_FAIRNESS 0
#endif

#include "gil.h"
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/*
 * GIL Performance Benchmark
 *
 * This benchmark measures the performance characteristics of the GIL
 * implementation under various contention scenarios. It's designed to
 * compare fastcond vs native pthread condition variable performance.
 *
 * Test scenarios:
 * 1. High contention - many threads competing frequently
 * 2. Burst mode - threads acquire/release in rapid bursts
 * 3. Mixed workload - variable hold times simulating real-world usage
 */

#define MAX_THREADS 32
#define MAX_SAMPLES 1000000

struct benchmark_context {
    struct fastcond_gil gil;
    volatile int active_threads;
    volatile int start_flag;
    volatile int stop_flag;

    // Configuration
    int num_threads;
    int iterations_per_thread;
    int hold_time_us;    // Microseconds to hold GIL
    int release_time_us; // Microseconds between acquisitions

    // Timing measurements (per thread)
    struct timespec *acquire_times;
    struct timespec *release_times;
    double *latencies; // Acquire latency in microseconds
    int sample_count;
    int max_samples;

    // Global statistics
    volatile long total_acquisitions;
    volatile long total_wait_time_ns;
    volatile long max_wait_time_ns;

    pthread_mutex_t stats_mutex;
};

// Get current time in nanoseconds
static inline long long get_time_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

// Calculate difference between two timespec values in microseconds
static double timespec_diff_us(struct timespec *start, struct timespec *end)
{
    return (end->tv_sec - start->tv_sec) * 1e6 + (end->tv_nsec - start->tv_nsec) * 1e-3;
}

// Simulate work with specified duration
static void busy_wait_us(int microseconds)
{
    if (microseconds <= 0)
        return;

    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    long long target_ns =
        start.tv_sec * 1000000000LL + start.tv_nsec + (long long) microseconds * 1000;

    do {
        clock_gettime(CLOCK_MONOTONIC, &now);
    } while (now.tv_sec * 1000000000LL + now.tv_nsec < target_ns);
}

void *benchmark_worker(void *arg)
{
    struct benchmark_context *ctx = (struct benchmark_context *) arg;
    struct timespec acquire_start, acquire_end, release_time;

    // Wait for start signal
    while (!ctx->start_flag && !ctx->stop_flag) {
        usleep(1000);
    }

    for (int i = 0; i < ctx->iterations_per_thread && !ctx->stop_flag; i++) {
        // Measure acquire latency
        clock_gettime(CLOCK_MONOTONIC, &acquire_start);
        fastcond_gil_acquire(&ctx->gil);
        clock_gettime(CLOCK_MONOTONIC, &acquire_end);

        // Calculate acquire latency
        double latency_us = timespec_diff_us(&acquire_start, &acquire_end);

        // Update global statistics
        __sync_add_and_fetch(&ctx->total_acquisitions, 1);

        long long wait_time_ns = (long long) (latency_us * 1000);
        __sync_add_and_fetch(&ctx->total_wait_time_ns, wait_time_ns);

        // Update max wait time
        long current_max;
        do {
            current_max = ctx->max_wait_time_ns;
        } while (current_max < wait_time_ns &&
                 !__sync_bool_compare_and_swap(&ctx->max_wait_time_ns, current_max, wait_time_ns));

        // Store sample if we have space (thread-safe sampling)
        pthread_mutex_lock(&ctx->stats_mutex);
        if (ctx->sample_count < ctx->max_samples) {
            int idx = ctx->sample_count++;
            ctx->acquire_times[idx] = acquire_start;
            ctx->release_times[idx] = acquire_end;
            ctx->latencies[idx] = latency_us;
        }
        pthread_mutex_unlock(&ctx->stats_mutex);

        // Simulate work while holding GIL
        busy_wait_us(ctx->hold_time_us);

        // Release GIL
        clock_gettime(CLOCK_MONOTONIC, &release_time);
        fastcond_gil_release(&ctx->gil);

        // Wait before next acquisition
        busy_wait_us(ctx->release_time_us);
    }

    __sync_sub_and_fetch(&ctx->active_threads, 1);
    return NULL;
}

static void print_latency_statistics(struct benchmark_context *ctx)
{
    if (ctx->sample_count == 0) {
        printf("No latency samples collected\n");
        return;
    }

    // Sort latencies for percentile calculation
    double *sorted_latencies = malloc(ctx->sample_count * sizeof(double));
    memcpy(sorted_latencies, ctx->latencies, ctx->sample_count * sizeof(double));

    // Simple bubble sort (adequate for small sample sizes)
    for (int i = 0; i < ctx->sample_count - 1; i++) {
        for (int j = 0; j < ctx->sample_count - i - 1; j++) {
            if (sorted_latencies[j] > sorted_latencies[j + 1]) {
                double temp = sorted_latencies[j];
                sorted_latencies[j] = sorted_latencies[j + 1];
                sorted_latencies[j + 1] = temp;
            }
        }
    }

    // Calculate statistics
    double sum = 0, sum_sq = 0;
    double min_lat = sorted_latencies[0];
    double max_lat = sorted_latencies[ctx->sample_count - 1];

    for (int i = 0; i < ctx->sample_count; i++) {
        sum += sorted_latencies[i];
        sum_sq += sorted_latencies[i] * sorted_latencies[i];
    }

    double mean = sum / ctx->sample_count;
    double variance = (sum_sq - sum * sum / ctx->sample_count) / (ctx->sample_count - 1);
    double std_dev = sqrt(variance);

    // Percentiles
    double p50 = sorted_latencies[ctx->sample_count / 2];
    double p90 = sorted_latencies[(int) (ctx->sample_count * 0.9)];
    double p95 = sorted_latencies[(int) (ctx->sample_count * 0.95)];
    double p99 = sorted_latencies[(int) (ctx->sample_count * 0.99)];

    printf("\n=== Latency Statistics ===\n");
    printf("Samples collected: %d\n", ctx->sample_count);
    printf("Min latency: %.2f μs\n", min_lat);
    printf("Max latency: %.2f μs\n", max_lat);
    printf("Mean latency: %.2f μs\n", mean);
    printf("Std deviation: %.2f μs\n", std_dev);
    printf("Percentiles:\n");
    printf("  50th: %.2f μs\n", p50);
    printf("  90th: %.2f μs\n", p90);
    printf("  95th: %.2f μs\n", p95);
    printf("  99th: %.2f μs\n", p99);

    free(sorted_latencies);
}

int run_benchmark(const char *test_name, int num_threads, int iterations_per_thread,
                  int hold_time_us, int release_time_us)
{
    struct benchmark_context ctx;
    pthread_t threads[MAX_THREADS];

    if (num_threads > MAX_THREADS) {
        fprintf(stderr, "Error: Maximum %d threads supported\n", MAX_THREADS);
        return 1;
    }

    printf("\n=== %s ===\n", test_name);
    printf("Backend: %s\n", FASTCOND_GIL_USE_NATIVE_COND ? "Native pthread" : "fastcond");
    printf("Fairness: %s\n", FASTCOND_GIL_DISABLE_FAIRNESS ? "DISABLED (plain mutex)" : "ENABLED");
    printf("Configuration: %d threads, %d iterations/thread\n", num_threads, iterations_per_thread);
    printf("Hold time: %d μs, Release time: %d μs\n", hold_time_us, release_time_us);

    // Initialize benchmark context
    memset(&ctx, 0, sizeof(ctx));
    fastcond_gil_init(&ctx.gil);
    pthread_mutex_init(&ctx.stats_mutex, NULL);

    ctx.num_threads = num_threads;
    ctx.iterations_per_thread = iterations_per_thread;
    ctx.hold_time_us = hold_time_us;
    ctx.release_time_us = release_time_us;
    ctx.active_threads = num_threads;
    ctx.max_samples = MAX_SAMPLES;

    // Allocate memory for timing data
    ctx.acquire_times = malloc(MAX_SAMPLES * sizeof(struct timespec));
    ctx.release_times = malloc(MAX_SAMPLES * sizeof(struct timespec));
    ctx.latencies = malloc(MAX_SAMPLES * sizeof(double));

    if (!ctx.acquire_times || !ctx.release_times || !ctx.latencies) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // Create worker threads
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, benchmark_worker, &ctx) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
    }

    printf("Starting benchmark...\n");
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Start the benchmark
    ctx.start_flag = 1;

    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // Calculate elapsed time
    double elapsed =
        (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) * 1e-9;

    printf("Benchmark completed in %.3f seconds\n", elapsed);

    // Print results
    printf("\n=== Performance Results ===\n");
    printf("Total acquisitions: %ld\n", ctx.total_acquisitions);
    printf("Acquisitions per second: %.0f\n", ctx.total_acquisitions / elapsed);
    printf("Average acquire latency: %.2f μs\n",
           (double) ctx.total_wait_time_ns / (ctx.total_acquisitions * 1000.0));
    printf("Maximum acquire latency: %.2f μs\n", ctx.max_wait_time_ns / 1000.0);

    // Detailed latency statistics
    print_latency_statistics(&ctx);

    // Cleanup
    fastcond_gil_destroy(&ctx.gil);
    pthread_mutex_destroy(&ctx.stats_mutex);
    free(ctx.acquire_times);
    free(ctx.release_times);
    free(ctx.latencies);

    return 0;
}

int main(int argc, char *argv[])
{
    printf("fastcond GIL Performance Benchmark\n");
    printf("===================================\n");

    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        printf("Usage: %s [num_threads] [iterations_per_thread]\n", argv[0]);
        printf("Runs predefined benchmark scenarios with specified parameters\n");
        return 0;
    }

    int num_threads = 4;
    int iterations = 10000;

    if (argc > 1) {
        num_threads = atoi(argv[1]);
        if (num_threads <= 0 || num_threads > MAX_THREADS) {
            fprintf(stderr, "Invalid number of threads. Must be 1-%d\n", MAX_THREADS);
            return 1;
        }
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations <= 0) {
            fprintf(stderr, "Invalid number of iterations\n");
            return 1;
        }
    }

    // Run multiple benchmark scenarios

    // 1. High contention - minimal hold time, immediate re-acquisition
    if (run_benchmark("High Contention Test", num_threads, iterations, 1, 1) != 0) {
        return 1;
    }

    // 2. Medium contention - moderate hold time
    if (run_benchmark("Medium Contention Test", num_threads, iterations, 10, 5) != 0) {
        return 1;
    }

    // 3. Low contention - longer hold times with gaps
    if (run_benchmark("Low Contention Test", num_threads, iterations, 50, 25) != 0) {
        return 1;
    }

    // 4. Burst mode - rapid acquire/release cycles
    if (run_benchmark("Burst Mode Test", num_threads * 2, iterations / 2, 0, 0) != 0) {
        return 1;
    }

    printf("\n=== Benchmark Suite Complete ===\n");
    printf("Backend tested: %s\n", FASTCOND_GIL_USE_NATIVE_COND ? "Native pthread" : "fastcond");
    printf("Fairness mode: %s\n",
           FASTCOND_GIL_DISABLE_FAIRNESS ? "DISABLED (plain mutex)" : "ENABLED");

    return 0;
}