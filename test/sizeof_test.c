/* Test to check structure sizes */

#include <stdio.h>
#include <pthread.h>
#include "fastcond.h"

#define FASTCOND_PATCH_COND
#include "fastcond_patch.h"

int main(void)
{
    setbuf(stdout, NULL);
    printf("sizeof(pthread_cond_t real) = %zu\n", sizeof(pthread_cond_t));
    printf("sizeof(fastcond_cond_t) = %zu\n", sizeof(fastcond_cond_t));
    printf("sizeof(fastcond_wcond_t) = %zu\n", sizeof(fastcond_wcond_t));
    
#ifdef FASTCOND_USE_GCD
    printf("Using GCD semaphores\n");
#else
    printf("Using POSIX semaphores\n");
#endif
    
    return 0;
}
