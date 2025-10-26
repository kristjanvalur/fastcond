/* Test atomic operations on all platforms */
#include "test_portability.h"
#include <stdio.h>

#define NUM_THREADS 4
#define INCREMENTS_PER_THREAD 10000

volatile int counter = 0;
volatile int max_seen = 0;

TEST_THREAD_FUNC_RETURN thread_func(void *arg)
{
    int thread_id = *(int *) arg;

    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        // Atomically increment counter
        int new_val = __sync_add_and_fetch(&counter, 1);

        // Track maximum value seen (should equal counter at this point)
        int current_max;
        do {
            current_max = max_seen;
        } while (current_max < new_val &&
                 !__sync_bool_compare_and_swap(&max_seen, current_max, new_val));

        // Atomically decrement counter
        __sync_sub_and_fetch(&counter, 1);
    }

    printf("Thread %d completed %d increments\n", thread_id, INCREMENTS_PER_THREAD);
    TEST_THREAD_RETURN;
}

int main(void)
{
    test_thread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    printf("Testing atomic operations with %d threads\n", NUM_THREADS);
    printf("Each thread will increment/decrement counter %d times\n", INCREMENTS_PER_THREAD);

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (test_thread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i);
            return 1;
        }
    }

    // Wait for completion
    for (int i = 0; i < NUM_THREADS; i++) {
        test_thread_join(threads[i], NULL);
    }

    printf("\n=== Results ===\n");
    printf("Final counter value: %d (should be 0)\n", counter);
    printf("Maximum value seen: %d (should be <= %d)\n", max_seen, NUM_THREADS);
    printf("Expected max: %d concurrent increments\n", NUM_THREADS);

    if (counter != 0) {
        printf("❌ FAIL: Counter not zero - atomic operations broken!\n");
        return 1;
    }

    if (max_seen > NUM_THREADS * 2) {
        printf("⚠️  WARNING: Max value suspiciously high - possible race condition\n");
    }

    printf("✅ PASS: Atomic operations working correctly\n");
    return 0;
}
