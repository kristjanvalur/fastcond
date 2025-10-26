/* Copyright (c) 2017-2025 Kristján Valur Jónsson */

/*
 * Test to verify that fastcond_patch.h correctly replaces native condition
 * variable functions with fastcond equivalents.
 *
 * This test validates that:
 * 1. The patch macros compile successfully
 * 2. Basic operations work (init, wait, signal, destroy)
 * 3. The patched code actually calls fastcond functions (verified via symbols)
 *
 * Platform support:
 * - POSIX: Replaces pthread_cond_* with fastcond_cond_*
 * - Windows: Replaces CONDITION_VARIABLE APIs with fastcond_*_wait_ms
 */

#include <stdio.h>
#include <stdlib.h>

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
    /* Windows: DeleteConditionVariable (non-standard) will be replaced by fastcond_*_fini via patch */
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
#ifdef _WIN32
    printf("\nTo verify fastcond symbols are used on Windows, check the binary\n");
    printf("or enable debugging to confirm fastcond_*_wait_ms is called.\n");
#else
    printf("\nTo verify fastcond symbols are used, run:\n");
    printf("   nm patch_test_fc | grep fastcond\n");
    printf("   (should see fastcond_cond_* or fastcond_wcond_* symbols)\n");
#endif

    return 0;
}
