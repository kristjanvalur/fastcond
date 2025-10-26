/* Minimal test to verify GIL mutual exclusion */
#include "gil.h"
#include "test_portability.h"
#include <stdio.h>

struct fastcond_gil gil;
volatile int holder_count = 0;
volatile int max_violation = 0;
volatile int acquisitions = 0;

#define MAX_ACQUISITIONS 10000

TEST_THREAD_FUNC_RETURN thread_func(void *arg)
{
    int thread_id = *(int *) arg;

    for (int i = 0; i < 100; i++) {
        /* Check if we've done enough total acquisitions */
        if (__sync_fetch_and_add(&acquisitions, 0) >= MAX_ACQUISITIONS) {
            break;
        }

        /* Acquire GIL */
        fastcond_gil_acquire(&gil);
        __sync_add_and_fetch(&acquisitions, 1);

        /* CRITICAL SECTION: Only one thread should be here */
        int current = __sync_add_and_fetch(&holder_count, 1);

        if (current > 1) {
            printf("❌ VIOLATION: Thread %d sees %d holders!\n", thread_id, current);

            /* Update max violation */
            int current_max;
            do {
                current_max = max_violation;
            } while (current_max < current &&
                     !__sync_bool_compare_and_swap(&max_violation, current_max, current));
        }

        /* Do some work */
        volatile int sum = 0;
        for (int j = 0; j < 100; j++) {
            sum += j;
        }

        /* Leave critical section */
        __sync_sub_and_fetch(&holder_count, 1);

        /* Release GIL */
        fastcond_gil_release(&gil);
    }

    printf("Thread %d finished\n", thread_id);
    TEST_THREAD_RETURN;
}

int main(void)
{
    printf("=== GIL Mutual Exclusion Test ===\n");
    printf("Backend: %s\n", FASTCOND_GIL_USE_NATIVE_COND ? "Native" : "fastcond");

    fastcond_gil_init(&gil);

    test_thread_t threads[4];
    int thread_ids[4] = {0, 1, 2, 3};

    for (int i = 0; i < 4; i++) {
        test_thread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }

    for (int i = 0; i < 4; i++) {
        test_thread_join(threads[i], NULL);
    }

    printf("\n=== Results ===\n");
    printf("Total acquisitions: %d\n", acquisitions);
    printf("Max holders seen: %d\n", max_violation);
    printf("Final holder_count: %d\n", holder_count);

    if (max_violation > 1) {
        printf("❌ FAILED: Mutual exclusion violated!\n");
        return 1;
    }

    if (holder_count != 0) {
        printf("❌ FAILED: Cleanup violation!\n");
        return 1;
    }

    printf("✅ PASSED: Mutual exclusion maintained\n");

    fastcond_gil_destroy(&gil);
    return 0;
}
