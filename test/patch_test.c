/* Copyright (c) 2017-2025 Kristján Valur Jónsson */

/*
 * Test to verify that fastcond_patch.h correctly replaces native condition
 * variable functions with fastcond equivalents.
 *
 * This test validates that:
 * 1. The patch macros compile successfully
 * 2. Basic operations work (init, wait, signal, destroy)
 * 3. The patched code actually calls fastcond functions (verified via callbacks)
 *
 * Platform support:
 * - POSIX: Replaces pthread_cond_* with fastcond_cond_*
 * - Windows: Replaces CONDITION_VARIABLE APIs with fastcond_*_wait_ms
 *
 * When compiled with FASTCOND_TEST_INSTRUMENTATION, uses callback mechanism
 * to definitively prove that fastcond functions are being called.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "native_primitives.h"
#include "test_portability.h"

/* Include fastcond.h first to get type declarations */
#include "fastcond.h"

/* Include the patch - this should replace native cond vars with fastcond */
#ifdef PATCH_WCOND
#define FASTCOND_PATCH_WCOND
#endif
#ifdef PATCH_COND
#define FASTCOND_PATCH_COND
#endif
#include "fastcond_patch.h"

#ifdef FASTCOND_TEST_INSTRUMENTATION
/* Callback tracking for instrumentation */
static int callback_count = 0;
static char last_function[64] = "";

static void test_callback(const char *function_name)
{
    callback_count++;
    strncpy(last_function, function_name, sizeof(last_function) - 1);
    last_function[sizeof(last_function) - 1] = '\0';
    printf("  [CALLBACK] %s called\n", function_name);
}
#endif /* FASTCOND_TEST_INSTRUMENTATION */

/* Test structure using "native" condition variable types that will be patched */
typedef struct {
    native_mutex_t mutex;
#ifdef _WIN32
    /* Windows: CONDITION_VARIABLE will be replaced by fastcond via patch */
    CONDITION_VARIABLE cond;
#else
    /* POSIX: pthread_cond_t will be replaced by fastcond via patch */
    pthread_cond_t cond;
#endif
    int ready;
    int done;
} test_ctx_t;

/* Simple thread that waits on condition variable */
TEST_THREAD_FUNC_RETURN waiter_thread(void *arg)
{
    test_ctx_t *ctx = (test_ctx_t *) arg;

    NATIVE_MUTEX_LOCK(&ctx->mutex);

    while (!ctx->ready) {
#ifdef _WIN32
        /* Windows: SleepConditionVariableCS will be replaced by fastcond via patch */
        SleepConditionVariableCS(&ctx->cond, &ctx->mutex, INFINITE);
#else
        /* POSIX: pthread_cond_wait will be replaced by fastcond via patch */
        pthread_cond_wait(&ctx->cond, &ctx->mutex);
#endif
    }

    ctx->done = 1;
    NATIVE_MUTEX_UNLOCK(&ctx->mutex);

    TEST_THREAD_RETURN;
}

int main(void)
{
    test_ctx_t ctx;
    test_thread_t thread;

#ifdef FASTCOND_TEST_INSTRUMENTATION
    /* Register callback to track fastcond calls */
    fastcond_set_test_callback(test_callback);
    callback_count = 0;
#endif

#ifdef _WIN32
    printf("Fastcond Patch Validation Test (Windows)\n");
    printf("=========================================\n");
#else
    printf("Fastcond Patch Validation Test (POSIX)\n");
    printf("=======================================\n");
#endif

#if defined(FASTCOND_PATCH_COND)
    printf("Patch mode: STRONG (fastcond_cond_t)\n");
#elif defined(FASTCOND_PATCH_WCOND)
    printf("Patch mode: WEAK (fastcond_wcond_t)\n");
#else
    printf("ERROR: No patch mode defined! Use -DPATCH_COND or -DPATCH_WCOND\n");
    return 1;
#endif

#ifdef FASTCOND_TEST_INSTRUMENTATION
    printf("Test instrumentation: ENABLED\n");
    printf("  (Callbacks will verify fastcond functions are called)\n");
#else
    printf("Test instrumentation: DISABLED\n");
    printf("  (To enable, compile with -DFASTCOND_TEST_INSTRUMENTATION)\n");
#endif
    printf("\n");

    printf("\nInitializing test context...\n");
    ctx.ready = 0;
    ctx.done = 0;
    /* Initialize test context */
    NATIVE_MUTEX_INIT(&ctx.mutex);

#ifdef _WIN32
    /* Windows: InitializeConditionVariable will be replaced by fastcond_*_init via patch */
    InitializeConditionVariable(&ctx.cond);
#else
    /* POSIX: pthread_cond_init will be replaced by fastcond_*_init via patch */
    pthread_cond_init(&ctx.cond, NULL);
#endif

    printf("Creating waiter thread...\n");
    if (test_thread_create(&thread, NULL, waiter_thread, &ctx) != 0) {
        fprintf(stderr, "Failed to create thread\n");
        return 1;
    }

    /* Give thread time to start waiting */
#ifdef _WIN32
    Sleep(100); /* 100ms */
#else
    usleep(100000); /* 100ms */
#endif

    printf("Signaling condition variable...\n");
    NATIVE_MUTEX_LOCK(&ctx.mutex);
    ctx.ready = 1;
#ifdef _WIN32
    /* Windows: WakeConditionVariable will be replaced by fastcond_*_signal via patch */
    WakeConditionVariable(&ctx.cond);
#else
    /* POSIX: pthread_cond_signal will be replaced by fastcond_*_signal via patch */
    pthread_cond_signal(&ctx.cond);
#endif
    NATIVE_MUTEX_UNLOCK(&ctx.mutex);

    printf("Waiting for thread to complete...\n");
    test_thread_join(thread, NULL);

    /* Verify thread completed */
    if (!ctx.done) {
        fprintf(stderr, "ERROR: Thread did not complete properly!\n");
        return 1;
    }

    printf("Cleaning up...\n");
#ifdef _WIN32
    /* Windows: DeleteConditionVariable (non-standard) will be replaced by fastcond_*_fini via patch
     */
    /* Note: Native CONDITION_VARIABLE has no cleanup, but fastcond does */
    DeleteConditionVariable(&ctx.cond);
#else
    /* POSIX: pthread_cond_destroy will be replaced by fastcond_*_fini via patch */
    pthread_cond_destroy(&ctx.cond);
#endif
    NATIVE_MUTEX_DESTROY(&ctx.mutex);

    printf("\n✅ Patch test PASSED\n");
    printf("   - init/wait/signal/destroy operations work\n");
    printf("   - Thread synchronized successfully\n");

#ifdef FASTCOND_TEST_INSTRUMENTATION
    printf("\n🔍 Instrumentation results:\n");
    printf("   - Total fastcond function calls: %d\n", callback_count);
    if (callback_count > 0) {
        printf("   - ✅ VERIFIED: Patched code calls fastcond functions\n");
        printf("   - Last function called: %s\n", last_function);
    } else {
        printf("   - ❌ WARNING: No fastcond calls detected!\n");
        printf("   - This suggests patching may not be working correctly\n");
        return 1;
    }
#ifdef _WIN32
    /* Windows should have called: init, wait_ms, signal */
    int expected_min = 3;
#else
    /* POSIX should have called: init, wait, signal */
    int expected_min = 3;
#endif
    if (callback_count >= expected_min) {
        printf("   - ✅ Expected number of calls confirmed (%d >= %d)\n", callback_count,
               expected_min);
    } else {
        printf("   - ⚠️  Fewer calls than expected (%d < %d)\n", callback_count, expected_min);
    }
#else
#ifdef _WIN32
    printf("\nTo verify fastcond symbols are used on Windows, check the binary\n");
    printf("or compile with -DFASTCOND_TEST_INSTRUMENTATION for definitive proof.\n");
#else
    printf("\nTo verify fastcond symbols are used, run:\n");
    printf("   nm patch_test_cond | grep fastcond\n");
    printf("   (should see fastcond_cond_* or fastcond_wcond_* symbols)\n");
    printf("Or compile with -DFASTCOND_TEST_INSTRUMENTATION for definitive proof.\n");
#endif
#endif /* FASTCOND_TEST_INSTRUMENTATION */

    return 0;
}
