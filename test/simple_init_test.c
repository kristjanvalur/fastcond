/* Simple test to verify basic init/fini works */

#include <stdio.h>
#include <pthread.h>
#include "fastcond.h"

#define FASTCOND_PATCH_COND
#include "fastcond_patch.h"

int main(void)
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    
    setbuf(stdout, NULL);
    printf("Starting simple init test\n");
    
    printf("Initializing mutex...\n");
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("ERROR: mutex init failed\n");
        return 1;
    }
    printf("Mutex initialized\n");
    
    printf("Initializing condition variable...\n");
    if (pthread_cond_init(&cond, NULL) != 0) {
        printf("ERROR: cond init failed\n");
        return 1;
    }
    printf("Condition variable initialized\n");
    
    printf("Destroying condition variable...\n");
    pthread_cond_destroy(&cond);
    printf("Condition variable destroyed\n");
    
    printf("Destroying mutex...\n");
    pthread_mutex_destroy(&mutex);
    printf("Mutex destroyed\n");
    
    printf("Test completed successfully\n");
    return 0;
}
