/* Copyright (c) 2017 Kristján Valur Jónsson */

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
 * Comprehensive GIL correctness and fairness test
 * 
 * This test validates:
 * 1. Mutual exclusion - only one thread holds GIL at a time
 * 2. Fairness statistics - measures how well the GIL prevents greedy re-acquisition
 * 3. Correctness under contention - multiple threads competing for GIL
 * 
 * Fairness is measured statistically rather than as an absolute guarantee,
 * as scheduling and timing can affect fairness in practice.
 */

#define MAX_THREADS 16
#define DEFAULT_ITERATIONS 10000
#define DEFAULT_WORK_CYCLES 1000

struct test_context {
    struct fastcond_gil gil;
    volatile int active_threads;
    volatile int holder_count;          // Should never exceed 1
    volatile int max_holder_violation;  // Track worst case violation
    
    // Thread startup synchronization
    pthread_mutex_t start_mutex;
    pthread_cond_t start_cond;
    volatile int threads_ready;         // Number of threads waiting to start
    
    // Fairness tracking - competitive acquisition model
    int thread_acquisitions[MAX_THREADS];
    pthread_t thread_ids[MAX_THREADS];
    int num_threads;
    int total_acquisitions_target;      // Total acquisitions to perform across all threads
    int hold_time_us;                   // Time to sleep while holding GIL
    int work_cycles;                    // Additional CPU work while holding GIL
    
    // Global acquisition counter
    volatile int global_acquisitions_done;
    
    // Acquisition sequence tracking for fairness analysis
    int *acquisition_sequence;          // Array of thread indices for each acquisition
    volatile int sequence_index;        // Current index in sequence array
    
    // Statistics
    volatile int consecutive_reacquisitions;
    volatile int max_consecutive_same_thread;
    volatile pthread_t last_holder;
    volatile int last_holder_count;
    
    // Test control
    volatile int start_flag;
    volatile int stop_flag;
};

// Simulate some work while holding the GIL
static void do_work_with_sleep(int cycles, int sleep_us) {
    // Do CPU work
    volatile int sum = 0;
    for (int i = 0; i < cycles; i++) {
        sum += i * i;
    }
    
    // Sleep while holding GIL to simulate I/O or other blocking operations
    if (sleep_us > 0) {
        usleep(sleep_us);
    }
}

void *worker_thread(void *arg) {
    struct test_context *ctx = (struct test_context *)arg;
    pthread_t self = pthread_self();
    int thread_idx = -1;
    
    // Find our thread index
    for (int i = 0; i < ctx->num_threads; i++) {
        if (NATIVE_THREAD_EQUAL(ctx->thread_ids[i], self)) {
            thread_idx = i;
            break;
        }
    }
    
    // Signal that this thread is ready and wait for synchronized start
    pthread_mutex_lock(&ctx->start_mutex);
    ctx->threads_ready++;
    if (ctx->threads_ready == ctx->num_threads) {
        // Last thread to arrive - signal all to start
        pthread_cond_broadcast(&ctx->start_cond);
    } else {
        // Wait for all threads to be ready
        while (ctx->threads_ready < ctx->num_threads && !ctx->stop_flag) {
            pthread_cond_wait(&ctx->start_cond, &ctx->start_mutex);
        }
    }
    pthread_mutex_unlock(&ctx->start_mutex);
    
    // Additional synchronization - wait for explicit start signal
    while (!ctx->start_flag && !ctx->stop_flag) {
        usleep(100);
    }
    
    int local_acquisitions = 0;
    
    // Compete for acquisitions until global target is reached
    while (!ctx->stop_flag) {
        // Check if we've reached the global target
        if (ctx->global_acquisitions_done >= ctx->total_acquisitions_target) {
            break;
        }
        
        // Acquire GIL
        fastcond_gil_acquire(&ctx->gil);
        
        // Double-check after acquiring (race condition possible)
        if (ctx->global_acquisitions_done >= ctx->total_acquisitions_target) {
            fastcond_gil_release(&ctx->gil);
            break;
        }
        
        // Increment global counter and record sequence
        int acquisition_number = __sync_add_and_fetch(&ctx->global_acquisitions_done, 1);
        
        // Record this thread's acquisition in the sequence (thread-safe)
        if (acquisition_number <= ctx->total_acquisitions_target) {
            int seq_idx = __sync_add_and_fetch(&ctx->sequence_index, 1) - 1;
            if (seq_idx < ctx->total_acquisitions_target) {
                ctx->acquisition_sequence[seq_idx] = thread_idx;
            }
        }
        
        // Critical section - verify mutual exclusion
        int current_holders = __sync_add_and_fetch(&ctx->holder_count, 1);
        if (current_holders > 1) {
            int violation = current_holders;
            // Update max violation atomically
            int current_max;
            do {
                current_max = ctx->max_holder_violation;
            } while (current_max < violation && 
                     !__sync_bool_compare_and_swap(&ctx->max_holder_violation, current_max, violation));
        }
        
        // Track fairness statistics
        local_acquisitions++;
        
        pthread_t prev_holder = ctx->last_holder;
        ctx->last_holder = self;
        
        if (NATIVE_THREAD_EQUAL(prev_holder, self)) {
            __sync_add_and_fetch(&ctx->consecutive_reacquisitions, 1);
            ctx->last_holder_count++;
            
            // Track maximum consecutive acquisitions by same thread
            int current_max;
            do {
                current_max = ctx->max_consecutive_same_thread;
            } while (current_max < ctx->last_holder_count && 
                     !__sync_bool_compare_and_swap(&ctx->max_consecutive_same_thread, 
                                                   current_max, ctx->last_holder_count));
        } else {
            ctx->last_holder_count = 1;
        }
        
        // Do work while holding the GIL (including sleep)
        do_work_with_sleep(ctx->work_cycles, ctx->hold_time_us);
        
        // Release GIL
        __sync_sub_and_fetch(&ctx->holder_count, 1);
        fastcond_gil_release(&ctx->gil);
        
        // Check if this was the last acquisition
        if (acquisition_number >= ctx->total_acquisitions_target) {
            break;
        }
    }
    
    if (thread_idx >= 0) {
        ctx->thread_acquisitions[thread_idx] = local_acquisitions;
    }
    
    __sync_sub_and_fetch(&ctx->active_threads, 1);
    return NULL;
}

static void print_fairness_statistics(struct test_context *ctx) {
    printf("\n=== Fairness Statistics ===\n");
    
    // Per-thread acquisition counts
    printf("Thread acquisition counts:\n");
    int min_acquisitions = ctx->thread_acquisitions[0];
    int max_acquisitions = ctx->thread_acquisitions[0];
    double sum = 0;
    
    for (int i = 0; i < ctx->num_threads; i++) {
        printf("  Thread %d: %d acquisitions\n", i, ctx->thread_acquisitions[i]);
        if (ctx->thread_acquisitions[i] < min_acquisitions) {
            min_acquisitions = ctx->thread_acquisitions[i];
        }
        if (ctx->thread_acquisitions[i] > max_acquisitions) {
            max_acquisitions = ctx->thread_acquisitions[i];
        }
        sum += ctx->thread_acquisitions[i];
    }
    
    double mean = sum / ctx->num_threads;
    double variance = 0;
    for (int i = 0; i < ctx->num_threads; i++) {
        double diff = ctx->thread_acquisitions[i] - mean;
        variance += diff * diff;
    }
    variance /= (ctx->num_threads - 1); // Bessel's correction
    double std_dev = sqrt(variance);
    double cv = std_dev / mean; // Coefficient of variation
    
    printf("\nDistribution statistics:\n");
    printf("  Mean acquisitions per thread: %.1f\n", mean);
    printf("  Standard deviation: %.1f\n", std_dev);
    printf("  Coefficient of variation: %.3f\n", cv);
    printf("  Min/Max acquisitions: %d/%d (ratio: %.2f)\n", 
           min_acquisitions, max_acquisitions, (double)max_acquisitions / min_acquisitions);
    
    // Acquisition sequence analysis
    printf("\nAcquisition sequence analysis:\n");
    if (ctx->sequence_index > 0) {
        // Count runs of same thread
        int max_run_length = 1;
        int current_run_length = 1;
        int total_runs = 1;
        int last_thread = ctx->acquisition_sequence[0];
        
        for (int i = 1; i < ctx->sequence_index && i < ctx->total_acquisitions_target; i++) {
            if (ctx->acquisition_sequence[i] == last_thread) {
                current_run_length++;
            } else {
                if (current_run_length > max_run_length) {
                    max_run_length = current_run_length;
                }
                total_runs++;
                current_run_length = 1;
                last_thread = ctx->acquisition_sequence[i];
            }
        }
        if (current_run_length > max_run_length) {
            max_run_length = current_run_length;
        }
        
        printf("  Total thread switches: %d\n", total_runs - 1);
        printf("  Average run length: %.1f\n", (double)ctx->sequence_index / total_runs);
        printf("  Maximum run length: %d\n", max_run_length);
        
        // Show first 20 acquisitions as example
        printf("  First 20 acquisitions by thread: ");
        int show_count = (ctx->sequence_index < 20) ? ctx->sequence_index : 20;
        for (int i = 0; i < show_count; i++) {
            printf("%d", ctx->acquisition_sequence[i]);
            if (i < show_count - 1) printf("-");
        }
        if (ctx->sequence_index > 20) printf("...");
        printf("\n");
        
        // Calculate transitions between different threads
        int transitions = 0;
        for (int i = 1; i < ctx->sequence_index && i < ctx->total_acquisitions_target; i++) {
            if (ctx->acquisition_sequence[i] != ctx->acquisition_sequence[i-1]) {
                transitions++;
            }
        }
        printf("  Thread transitions: %d out of %d acquisitions (%.1f%%)\n", 
               transitions, ctx->sequence_index, 
               100.0 * transitions / (ctx->sequence_index - 1));
    }
    
    printf("\nFairness metrics:\n");
    printf("  Total acquisitions: %d\n", ctx->global_acquisitions_done);
    printf("  Consecutive re-acquisitions: %d (%.1f%%)\n", 
           ctx->consecutive_reacquisitions, 
           100.0 * ctx->consecutive_reacquisitions / ctx->global_acquisitions_done);
    printf("  Max consecutive by same thread: %d\n", ctx->max_consecutive_same_thread);
    
    // Fairness score (lower is better, 0 = perfect fairness)
    double fairness_score = cv * 100; // Scale coefficient of variation
    printf("  Fairness score: %.1f (lower is better)\n", fairness_score);
    
    if (fairness_score < 5.0) {
        printf("  Assessment: Excellent fairness\n");
    } else if (fairness_score < 15.0) {
        printf("  Assessment: Good fairness\n");
    } else if (fairness_score < 30.0) {
        printf("  Assessment: Moderate fairness\n");
    } else {
        printf("  Assessment: Poor fairness - may indicate greedy behaviour\n");
    }
}

int run_gil_test(int num_threads, int total_acquisitions, int hold_time_us, int work_cycles) {
    struct test_context ctx;
    pthread_t threads[MAX_THREADS];
    
    if (num_threads > MAX_THREADS) {
        fprintf(stderr, "Error: Maximum %d threads supported\n", MAX_THREADS);
        return 1;
    }
    
    printf("=== GIL Correctness and Fairness Test ===\n");
    printf("Backend: %s\n", FASTCOND_GIL_USE_NATIVE_COND ? "Native pthread" : "fastcond");
    printf("Fairness: %s\n", FASTCOND_GIL_DISABLE_FAIRNESS ? "DISABLED (plain mutex)" : "ENABLED");
    printf("Configuration: %d threads competing for %d total acquisitions\n", 
           num_threads, total_acquisitions);
    printf("Hold time: %d μs, Work cycles: %d\n", hold_time_us, work_cycles);
    
    // Initialize test context
    memset(&ctx, 0, sizeof(ctx));
    fastcond_gil_init(&ctx.gil);
    pthread_mutex_init(&ctx.start_mutex, NULL);
    pthread_cond_init(&ctx.start_cond, NULL);
    
    ctx.num_threads = num_threads;
    ctx.total_acquisitions_target = total_acquisitions;
    ctx.hold_time_us = hold_time_us;
    ctx.work_cycles = work_cycles;
    ctx.active_threads = num_threads;
    
    // Allocate acquisition sequence tracking array
    ctx.acquisition_sequence = malloc(total_acquisitions * sizeof(int));
    if (!ctx.acquisition_sequence) {
        fprintf(stderr, "Memory allocation failed for acquisition sequence\n");
        return 1;
    }
    
    printf("Creating %d threads...\n", num_threads);
    
    // Create worker threads
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, worker_thread, &ctx) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
        ctx.thread_ids[i] = threads[i];
    }
    
    // Wait for all threads to signal readiness
    pthread_mutex_lock(&ctx.start_mutex);
    while (ctx.threads_ready < num_threads) {
        pthread_cond_wait(&ctx.start_cond, &ctx.start_mutex);
    }
    pthread_mutex_unlock(&ctx.start_mutex);
    
    printf("All threads ready. Starting synchronized test...\n");
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    // Signal all threads to start simultaneously
    pthread_mutex_lock(&ctx.start_mutex);
    ctx.start_flag = 1;
    pthread_cond_broadcast(&ctx.start_cond);
    pthread_mutex_unlock(&ctx.start_mutex);
    
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    // Calculate elapsed time
    double elapsed = (end_time.tv_sec - start_time.tv_sec) + 
                    (end_time.tv_nsec - start_time.tv_nsec) * 1e-9;
    
    printf("Test completed in %.3f seconds\n", elapsed);
    
    // Check for correctness violations
    printf("\n=== Correctness Results ===\n");
    if (ctx.max_holder_violation > 1) {
        printf("❌ MUTUAL EXCLUSION VIOLATION: Up to %d threads held GIL simultaneously!\n", 
               ctx.max_holder_violation);
    } else {
        printf("✅ Mutual exclusion: PASSED (max holders: %d)\n", ctx.max_holder_violation);
    }
    
    if (ctx.holder_count != 0) {
        printf("❌ CLEANUP VIOLATION: %d threads still holding GIL after test\n", 
               ctx.holder_count);
    } else {
        printf("✅ Cleanup: PASSED (no threads holding GIL)\n");
    }
    
    // Print fairness statistics
    print_fairness_statistics(&ctx);
    
    // Performance metrics
    printf("\n=== Performance Metrics ===\n");
    printf("Total acquire/release cycles: %d\n", ctx.global_acquisitions_done);
    printf("Operations per second: %.0f\n", ctx.global_acquisitions_done / elapsed);
    printf("Average latency per operation: %.1f μs\n", 
           (elapsed * 1e6) / ctx.global_acquisitions_done);
    
    // Cleanup
    fastcond_gil_destroy(&ctx.gil);
    pthread_mutex_destroy(&ctx.start_mutex);
    pthread_cond_destroy(&ctx.start_cond);
    free(ctx.acquisition_sequence);
    
    return (ctx.max_holder_violation > 1) ? 1 : 0;
}

int main(int argc, char *argv[]) {
    int num_threads = 4;
    int total_acquisitions = DEFAULT_ITERATIONS;
    int hold_time_us = 100;  // 100 microseconds default hold time
    int work_cycles = DEFAULT_WORK_CYCLES;
    
    // Parse command line arguments
    if (argc > 1) {
        num_threads = atoi(argv[1]);
        if (num_threads <= 0 || num_threads > MAX_THREADS) {
            fprintf(stderr, "Invalid number of threads. Must be 1-%d\n", MAX_THREADS);
            return 1;
        }
    }
    if (argc > 2) {
        total_acquisitions = atoi(argv[2]);
        if (total_acquisitions <= 0) {
            fprintf(stderr, "Invalid number of total acquisitions\n");
            return 1;
        }
    }
    if (argc > 3) {
        hold_time_us = atoi(argv[3]);
        if (hold_time_us < 0) {
            fprintf(stderr, "Invalid hold time\n");
            return 1;
        }
    }
    if (argc > 4) {
        work_cycles = atoi(argv[4]);
        if (work_cycles < 0) {
            fprintf(stderr, "Invalid work cycles\n");
            return 1;
        }
    }
    
    printf("fastcond GIL Test Suite\n");
    printf("Usage: %s [num_threads] [total_acquisitions] [hold_time_us] [work_cycles]\n\n", argv[0]);
    
    return run_gil_test(num_threads, total_acquisitions, hold_time_us, work_cycles);
}