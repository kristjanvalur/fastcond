/* Copyright (c) 2017-2025 Kristján Valur Jónsson */

// Fairness mechanism control - matches definition in gil.c
#ifndef FASTCOND_GIL_DISABLE_FAIRNESS
#define FASTCOND_GIL_DISABLE_FAIRNESS 0
#endif

#include "gil.h"
#include "test_portability.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Comprehensive GIL correctness and fairness test with Python-like behavior simulation
 *
 * This test validates:
 * 1. Mutual exclusion - only one thread holds GIL at a time
 * 2. Fairness statistics - measures how well the GIL prevents greedy re-acquisition
 * 3. Correctness under contention - multiple threads competing for GIL
 * 4. Python-like execution pattern - cooperative yields with occasional I/O
 *
 * NEW PYTHON-LIKE BEHAVIOR PATTERN:
 * - Threads initialize by acquiring GIL (like Python thread startup)
 * - Most iterations use fastcond_gil_yield() for cooperative fairness
 * - Occasionally (~10% probability) threads do I/O: release GIL + delay + reacquire
 * - Threads finalize by releasing GIL (like Python thread shutdown)
 *
 * This better simulates real interpreter behavior where threads mostly yield
 * for fairness but occasionally perform I/O operations that release the GIL.
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
    volatile int holder_count;         // Should never exceed 1
    volatile int max_holder_violation; // Track worst case violation

    // Thread startup synchronization
    test_mutex_t start_mutex;
    test_cond_t start_cond;
    volatile int threads_ready; // Number of threads waiting to start

    // Fairness tracking - competitive acquisition model
    int thread_acquisitions[MAX_THREADS];
    test_thread_t thread_ids[MAX_THREADS];
    int num_threads;
    int total_acquisitions_target; // Total acquisitions to perform across all threads
    int hold_time_us;              // Time to sleep while holding GIL
    int work_cycles;               // Additional CPU work while holding GIL
    int release_delay_us;          // Time to sleep after releasing GIL (simulates I/O)
    int release_delay_variance_us; // Random variance in release delay (±variance)

    // Global acquisition counter
    volatile int global_acquisitions_done;

    // Acquisition sequence tracking for fairness analysis
    int *acquisition_sequence;   // Array of thread indices for each acquisition
    volatile int sequence_index; // Current index in sequence array

    // Statistics
    volatile int consecutive_reacquisitions;
    volatile int max_consecutive_same_thread;
    volatile test_thread_t last_holder;
    volatile int last_holder_count;

    // Test control
    volatile int start_flag;
    volatile int stop_flag;
};

// Simulate some work while holding the GIL
static void do_work_with_sleep(int cycles, int sleep_us)
{
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

// Thread wrapper to pass both context and thread index
struct thread_arg {
    struct test_context *ctx;
    int thread_idx;
};

TEST_THREAD_FUNC_RETURN worker_thread(void *arg)
{
    struct thread_arg *targ = (struct thread_arg *) arg;
    struct test_context *ctx = targ->ctx;
    int thread_idx = targ->thread_idx;
    test_thread_t self = test_thread_self();

    // Signal that this thread is ready and wait for synchronized start
    test_mutex_lock(&ctx->start_mutex);
    ctx->threads_ready++;

    // Signal main thread that we've incremented the counter
    test_cond_broadcast(&ctx->start_cond);

    if (ctx->threads_ready == ctx->num_threads) {
        // Last thread to arrive - all threads are ready
        // (main thread will see threads_ready == num_threads and proceed)
    } else {
        // Wait for all threads to be ready
        while (ctx->threads_ready < ctx->num_threads && !ctx->stop_flag) {
            test_cond_wait(&ctx->start_cond, &ctx->start_mutex);
        }
    }
    test_mutex_unlock(&ctx->start_mutex);

    // Wait for explicit start signal using condition variable
    test_mutex_lock(&ctx->start_mutex);
    while (!ctx->start_flag && !ctx->stop_flag) {
        test_cond_wait(&ctx->start_cond, &ctx->start_mutex);
    }
    test_mutex_unlock(&ctx->start_mutex);

    int local_acquisitions = 0;

    // INITIALIZE: Acquire GIL at thread startup (Python-like behavior)
    fastcond_gil_acquire(&ctx->gil);
    int yields_since_io = 0;
    const int IO_PROBABILITY = 10; // Do I/O every ~10 yields on average

    // Python-like execution pattern: mostly yields with occasional I/O
    while (!ctx->stop_flag) {
        // Check if we've reached the global target
        if (__sync_fetch_and_add(&ctx->global_acquisitions_done, 0) >=
            ctx->total_acquisitions_target) {
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
            } while (
                current_max < violation &&
                !__sync_bool_compare_and_swap(&ctx->max_holder_violation, current_max, violation));
        }

        // Track fairness statistics
        local_acquisitions++;

        test_thread_t prev_holder = ctx->last_holder;
        ctx->last_holder = self;

        if (test_thread_equal(prev_holder, self)) {
            __sync_add_and_fetch(&ctx->consecutive_reacquisitions, 1);
            ctx->last_holder_count++;

            // Track maximum consecutive acquisitions by same thread
            int current_max;
            do {
                current_max = ctx->max_consecutive_same_thread;
            } while (current_max < ctx->last_holder_count &&
                     !__sync_bool_compare_and_swap(&ctx->max_consecutive_same_thread, current_max,
                                                   ctx->last_holder_count));
        } else {
            ctx->last_holder_count = 1;
        }

        // Do work while holding the GIL
        do_work_with_sleep(ctx->work_cycles, ctx->hold_time_us);

        // Check if we should stop before yielding/doing I/O
        if (acquisition_number >= ctx->total_acquisitions_target) {
            // End critical section before exiting
            __sync_sub_and_fetch(&ctx->holder_count, 1);
            break;
        }

        // End critical section - decrement before giving up execution
        __sync_sub_and_fetch(&ctx->holder_count, 1);

        // CORE PYTHON-LIKE BEHAVIOR: Decide between yield vs I/O
        yields_since_io++;
        int should_do_io = (rand() % IO_PROBABILITY == 0) || yields_since_io >= IO_PROBABILITY;

        if (should_do_io) {
            // SIMULATE I/O: Release GIL, do I/O work, then reacquire
            fastcond_gil_release(&ctx->gil);

            // Simulate I/O operation with configured delay and variance
            int delay = ctx->release_delay_us;
            if (ctx->release_delay_variance_us > 0) {
                int variance = (rand() % (2 * ctx->release_delay_variance_us + 1)) -
                               ctx->release_delay_variance_us;
                delay = ctx->release_delay_us + variance;
            }

            // Only sleep if delay is positive (negative delay = immediate reacquisition)
            if (delay > 0) {
                usleep(delay);
            }

            // Reacquire GIL after I/O
            fastcond_gil_acquire(&ctx->gil);

            yields_since_io = 0; // Reset counter
            // Next loop iteration will re-enter critical section with holder_count++
        } else {
            // REGULAR YIELD: Cooperative yielding for fairness
            // yield() maintains GIL ownership but gives other threads a turn
            fastcond_gil_yield(&ctx->gil);
            // Next loop iteration will re-enter critical section with holder_count++
        }
    }

    // FINALIZE: Release GIL at thread shutdown (Python-like behavior)
    fastcond_gil_release(&ctx->gil);

    if (thread_idx >= 0) {
        ctx->thread_acquisitions[thread_idx] = local_acquisitions;
    }

    __sync_sub_and_fetch(&ctx->active_threads, 1);
    TEST_THREAD_RETURN;
}

static void print_fairness_statistics(struct test_context *ctx, int num_threads)
{
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
    printf("  Min/Max acquisitions: %d/%d (ratio: %.2f)\n", min_acquisitions, max_acquisitions,
           (double) max_acquisitions / min_acquisitions);

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
        printf("  Average run length: %.1f\n", (double) ctx->sequence_index / total_runs);
        printf("  Maximum run length: %d\n", max_run_length);

        // Show first 20 acquisitions as example
        printf("  First 20 acquisitions by thread: ");
        int show_count = (ctx->sequence_index < 20) ? ctx->sequence_index : 20;
        for (int i = 0; i < show_count; i++) {
            printf("%d", ctx->acquisition_sequence[i]);
            if (i < show_count - 1)
                printf("-");
        }
        if (ctx->sequence_index > 20)
            printf("...");
        printf("\n");

        // Calculate transitions between different threads
        int transitions = 0;
        for (int i = 1; i < ctx->sequence_index && i < ctx->total_acquisitions_target; i++) {
            if (ctx->acquisition_sequence[i] != ctx->acquisition_sequence[i - 1]) {
                transitions++;
            }
        }
        printf("  Thread transitions: %d out of %d acquisitions (%.1f%%)\n", transitions,
               ctx->sequence_index, 100.0 * transitions / (ctx->sequence_index - 1));

        // Inter-acquisition wait depth analysis
        printf("\n=== Inter-Acquisition Wait Depth Analysis ===\n");
        printf("For each acquisition, count how many other threads acquired GIL since this "
               "thread's previous acquisition\n");

        // Track last acquisition index for each thread
        int *last_acquisition_index = calloc(num_threads, sizeof(int));
        int *wait_depths = malloc((ctx->sequence_index + num_threads) *
                                  sizeof(int)); // Extra space for terminal waits
        int total_wait_depth_samples = 0;
        long long total_wait_depth = 0;
        long long total_wait_depth_squared = 0;
        int max_wait_depth = 0;

        // Initialize: first acquisition for each thread has wait depth 0
        for (int i = 0; i < num_threads; i++) {
            last_acquisition_index[i] = -1;
        }

        // Process each acquisition in sequence
        for (int i = 0; i < ctx->sequence_index && i < ctx->total_acquisitions_target; i++) {
            int thread_id = ctx->acquisition_sequence[i];
            int wait_depth = 0;

            if (last_acquisition_index[thread_id] != -1) {
                // Count acquisitions by other threads since this thread's last acquisition
                wait_depth = i - last_acquisition_index[thread_id] - 1;
                wait_depths[total_wait_depth_samples] = wait_depth;
                total_wait_depth += wait_depth;
                total_wait_depth_squared += (long long) wait_depth * wait_depth;
                total_wait_depth_samples++;

                if (wait_depth > max_wait_depth) {
                    max_wait_depth = wait_depth;
                }
            }

            last_acquisition_index[thread_id] = i;
        }

        // Add terminal wait depths for each thread
        // This captures the wait depth that starved threads would experience
        for (int thread_id = 0; thread_id < num_threads; thread_id++) {
            if (last_acquisition_index[thread_id] != -1) {
                // Calculate wait depth from this thread's last acquisition to sequence end
                int terminal_wait_depth =
                    ctx->sequence_index - last_acquisition_index[thread_id] - 1;
                if (terminal_wait_depth > 0) {
                    wait_depths[total_wait_depth_samples] = terminal_wait_depth;
                    total_wait_depth += terminal_wait_depth;
                    total_wait_depth_squared +=
                        (long long) terminal_wait_depth * terminal_wait_depth;
                    total_wait_depth_samples++;

                    if (terminal_wait_depth > max_wait_depth) {
                        max_wait_depth = terminal_wait_depth;
                    }
                }
            } else {
                // Thread never acquired GIL - would wait through entire sequence
                if (ctx->sequence_index > 0) {
                    int full_wait_depth = ctx->sequence_index;
                    wait_depths[total_wait_depth_samples] = full_wait_depth;
                    total_wait_depth += full_wait_depth;
                    total_wait_depth_squared += (long long) full_wait_depth * full_wait_depth;
                    total_wait_depth_samples++;

                    if (full_wait_depth > max_wait_depth) {
                        max_wait_depth = full_wait_depth;
                    }
                }
            }
        }

        if (total_wait_depth_samples > 0) {
            double mean_wait_depth = (double) total_wait_depth / total_wait_depth_samples;
            double variance = (double) total_wait_depth_squared / total_wait_depth_samples -
                              mean_wait_depth * mean_wait_depth;
            double std_dev = sqrt(variance > 0 ? variance : 0);

            printf("Wait depth statistics:\n");
            printf("  Samples: %d acquisitions (excluding first per thread)\n",
                   total_wait_depth_samples);
            printf("  Mean wait depth: %.2f acquisitions\n", mean_wait_depth);
            printf("  Standard deviation: %.2f\n", std_dev);
            printf("  Max wait depth: %d acquisitions\n", max_wait_depth);
            printf("  Expected for fair scheduler: %.1f (n-1 where n=%d threads)\n",
                   (double) (num_threads - 1), num_threads);

            // Distribution analysis
            printf("\nWait depth distribution:\n");
            int *histogram = calloc(max_wait_depth + 1, sizeof(int));
            for (int i = 0; i < total_wait_depth_samples; i++) {
                histogram[wait_depths[i]]++;
            }

            // Show distribution for first 10 values or all if fewer
            int show_max = (max_wait_depth > 9) ? 9 : max_wait_depth;
            for (int i = 0; i <= show_max; i++) {
                double percentage = 100.0 * histogram[i] / total_wait_depth_samples;
                printf("  %d other acquisitions: %d samples (%.1f%%)\n", i, histogram[i],
                       percentage);
            }
            if (max_wait_depth > 9) {
                printf("  ... (showing first 10 buckets)\n");
            }

            // Theoretical analysis
            printf("\nTheoretical analysis:\n");
            if (std_dev < 0.1) {
                double expected_fair = (double) (num_threads - 1);
                if (mean_wait_depth > expected_fair - 0.1 &&
                    mean_wait_depth < expected_fair + 0.1) {
                    printf("  Pattern: Deterministic round-robin (all values = n-1)\n");
                    printf("  Assessment: Perfect round-robin scheduling\n");
                } else if (mean_wait_depth < 1.0) {
                    printf("  Pattern: Deterministic greedy (all values near 0)\n");
                    printf("  Assessment: Thread starvation - same thread re-acquiring\n");
                } else {
                    printf("  Pattern: Deterministic (all values identical = %.1f)\n",
                           mean_wait_depth);
                    printf("  Assessment: Regular but non-round-robin pattern\n");
                }
            } else if (mean_wait_depth > num_threads - 1 + 2 * std_dev) {
                printf("  Pattern: Heavy-tailed distribution\n");
                printf("  Assessment: Some threads experience long waits (unfair)\n");
            } else if (std_dev > mean_wait_depth * 0.8) {
                printf("  Pattern: High variance relative to mean\n");
                printf("  Assessment: Random/unpredictable scheduling\n");
            } else {
                printf("  Pattern: Low variance around expected value\n");
                printf("  Assessment: Well-controlled fair scheduling\n");
            }

            free(histogram);
        } else {
            printf("  No wait depth data available (insufficient acquisitions)\n");
        }

        free(last_acquisition_index);
        free(wait_depths);
    }

    printf("\nFairness metrics:\n");
    printf("  Total acquisitions: %d\n", ctx->global_acquisitions_done);
    printf("  Consecutive re-acquisitions: %d (%.1f%%)\n", ctx->consecutive_reacquisitions,
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

int run_gil_test(int num_threads, int total_acquisitions, int hold_time_us, int work_cycles,
                 int release_delay_us, int release_delay_variance_us)
{
    struct test_context ctx;
    test_thread_t threads[MAX_THREADS];
    struct thread_arg thread_args[MAX_THREADS]; // Pass index safely

    if (num_threads > MAX_THREADS) {
        fprintf(stderr, "Error: Maximum %d threads supported\n", MAX_THREADS);
        return 1;
    }

    printf("=== GIL Correctness and Fairness Test ===\n");
    printf("Backend: %s\n", FASTCOND_GIL_USE_NATIVE_COND ? "Native pthread" : "fastcond");
    printf("Fairness: %s\n", FASTCOND_GIL_DISABLE_FAIRNESS ? "DISABLED (plain mutex)" : "ENABLED");
    printf("Configuration: %d threads competing for %d total acquisitions\n", num_threads,
           total_acquisitions);
    printf("Hold time: %d μs, Work cycles: %d, Release delay: %d±%d μs", hold_time_us, work_cycles,
           release_delay_us, release_delay_variance_us);
    if (release_delay_us < 0) {
        printf(" (mostly instant, occasional I/O)");
    }
    printf("\n");

    // Initialize test context
    memset(&ctx, 0, sizeof(ctx));
    fastcond_gil_init(&ctx.gil);
    test_mutex_init(&ctx.start_mutex, NULL);
    test_cond_init(&ctx.start_cond, NULL);

    ctx.num_threads = num_threads;
    ctx.total_acquisitions_target = total_acquisitions;
    ctx.hold_time_us = hold_time_us;
    ctx.work_cycles = work_cycles;
    ctx.release_delay_us = release_delay_us;
    ctx.release_delay_variance_us = release_delay_variance_us;
    ctx.active_threads = num_threads;

    // Allocate acquisition sequence tracking array
    ctx.acquisition_sequence = malloc(total_acquisitions * sizeof(int));
    if (!ctx.acquisition_sequence) {
        fprintf(stderr, "Memory allocation failed for acquisition sequence\n");
        return 1;
    }

    // Create worker threads with proper index passing
    for (int i = 0; i < num_threads; i++) {
        thread_args[i].ctx = &ctx;
        thread_args[i].thread_idx = i;

        if (test_thread_create(&threads[i], NULL, worker_thread, &thread_args[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
        ctx.thread_ids[i] = threads[i]; // Store for cleanup only
    }

    // Wait for all threads to signal readiness
    test_mutex_lock(&ctx.start_mutex);

    while (ctx.threads_ready < num_threads) {
        test_cond_wait(&ctx.start_cond, &ctx.start_mutex);
    }
    test_mutex_unlock(&ctx.start_mutex);

    // Start the test
    test_timespec_t start_time, end_time;
    test_clock_gettime(&start_time);

    // Signal all threads to start simultaneously
    test_mutex_lock(&ctx.start_mutex);
    ctx.start_flag = 1;
    test_cond_broadcast(&ctx.start_cond);
    test_mutex_unlock(&ctx.start_mutex);

    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        test_thread_join(threads[i], NULL);
    }

    test_clock_gettime(&end_time);

    // Calculate elapsed time
    double elapsed = test_timespec_diff(&end_time, &start_time);

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
        printf("❌ CLEANUP VIOLATION: %d threads still holding GIL after test\n", ctx.holder_count);
    } else {
        printf("✅ Cleanup: PASSED (no threads holding GIL)\n");
    }

    // Print fairness statistics
    print_fairness_statistics(&ctx, num_threads);

    // Performance metrics
    printf("\n=== Performance Metrics ===\n");
    printf("Total acquire/release cycles: %d\n", ctx.global_acquisitions_done);
    printf("Operations per second: %.0f\n", ctx.global_acquisitions_done / elapsed);
    printf("Average latency per operation: %.1f μs\n",
           (elapsed * 1e6) / ctx.global_acquisitions_done);

    // Cleanup
    fastcond_gil_destroy(&ctx.gil);
    test_mutex_destroy(&ctx.start_mutex);
    test_cond_destroy(&ctx.start_cond);
    free(ctx.acquisition_sequence);

    return (ctx.max_holder_violation > 1) ? 1 : 0;
}

// Simple test to verify fastcond_gil_yield() functionality
void test_gil_yield()
{
    printf("\n=== GIL Yield API Test ===\n");

    struct fastcond_gil gil;
    fastcond_gil_init(&gil);

    // Acquire GIL
    fastcond_gil_acquire(&gil);
    printf("  ✅ Acquired GIL\n");

    // Test yield - should be a no-op when no waiters
    fastcond_gil_yield(&gil);
    printf("  ✅ Yielded GIL (no waiters)\n");

    // Release GIL
    fastcond_gil_release(&gil);
    printf("  ✅ Released GIL\n");

    fastcond_gil_destroy(&gil);
    printf("GIL yield API test completed successfully!\n");
}

int main(int argc, char *argv[])
{
    int num_threads = 8; // Increased for better statistical power (was 4)
    int total_acquisitions = DEFAULT_ITERATIONS;
    int hold_time_us = 100; // 100 microseconds default hold time
    int work_cycles = DEFAULT_WORK_CYCLES;
    int release_delay_us = 1000;       // 1ms default release delay (simulates I/O)
    int release_delay_variance_us = 0; // No variance by default

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
    if (argc > 5) {
        release_delay_us = atoi(argv[5]);
        // Allow negative delays to simulate Python GIL timer behavior
    }
    if (argc > 6) {
        release_delay_variance_us = atoi(argv[6]);
        if (release_delay_variance_us < 0) {
            fprintf(stderr, "Invalid release delay variance\n");
            return 1;
        }
    }

    printf("fastcond GIL Test Suite\n");
    printf("Usage: %s [num_threads] [total_acquisitions] [hold_time_us] [work_cycles] "
           "[release_delay_us] [release_delay_variance_us]\n\n",
           argv[0]);

    // Run yield API test first
    test_gil_yield();

    // Initialize random seed for release delay variance
    srand(time(NULL));

    return run_gil_test(num_threads, total_acquisitions, hold_time_us, work_cycles,
                        release_delay_us, release_delay_variance_us);
}