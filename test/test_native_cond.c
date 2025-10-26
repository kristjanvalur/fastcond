/* Simple test to verify CRITICAL_SECTION + CONDITION_VARIABLE work correctly */
#include "native_primitives.h"
#include "test_portability.h"
#include <stdio.h>

native_mutex_t mutex;
native_cond_t cond;
volatile int ready = 0;
volatile int thread_in_critical_section = 0;

TEST_THREAD_FUNC_RETURN thread_func(void *arg)
{
    printf("Thread: Locking mutex...\n");
    NATIVE_MUTEX_LOCK(&mutex);
    printf("Thread: Mutex locked, incrementing counter...\n");
    thread_in_critical_section++;

    if (thread_in_critical_section > 1) {
        printf("ERROR: Multiple threads in critical section! Count=%d\n",
               thread_in_critical_section);
    }

    printf("Thread: Waiting on condition (should release mutex)...\n");
    while (!ready) {
        NATIVE_COND_WAIT(&cond, &mutex);
    }
    printf("Thread: Woken up, checking counter...\n");

    if (thread_in_critical_section != 1) {
        printf("ERROR: Counter changed while waiting! Count=%d\n", thread_in_critical_section);
    }

    thread_in_critical_section--;
    printf("Thread: Unlocking mutex\n");
    NATIVE_MUTEX_UNLOCK(&mutex);
    TEST_THREAD_RETURN;
}

int main(void)
{
    test_thread_t thread;

    printf("Initializing mutex and condition variable...\n");
    NATIVE_MUTEX_INIT(&mutex);
    NATIVE_COND_INIT(&cond);

    printf("Creating thread...\n");
    test_thread_create(&thread, NULL, thread_func, NULL);

    /* Give thread time to start waiting */
    usleep(200000);

    printf("Main: Locking mutex...\n");
    NATIVE_MUTEX_LOCK(&mutex);
    printf("Main: Mutex locked (thread should have released it)\n");

    /* Thread incremented to 1 and is waiting. If we successfully got the mutex,
     * counter should still be 1 (thread released mutex during wait). */
    if (thread_in_critical_section != 1) {
        printf("ERROR: Unexpected counter value! Count=%d (expected 1)\n",
               thread_in_critical_section);
        printf("  This suggests thread never reached wait, or mutex is broken\n");
        return 1;
    }

    printf("Main: Good! Counter is 1, thread released mutex properly\n");

    printf("Main: Signaling condition...\n");
    ready = 1;
    NATIVE_COND_SIGNAL(&cond);

    printf("Main: Unlocking mutex\n");
    NATIVE_MUTEX_UNLOCK(&mutex);

    printf("Main: Waiting for thread to finish...\n");
    test_thread_join(thread, NULL);

    NATIVE_COND_DESTROY(&cond);
    NATIVE_MUTEX_DESTROY(&mutex);

    printf("✅ Test PASSED - mutex release/reacquire works correctly\n");
    return 0;
}
