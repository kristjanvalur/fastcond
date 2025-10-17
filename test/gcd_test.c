/* Direct GCD semaphore test - no fastcond involved */

#include <stdio.h>
#include <dispatch/dispatch.h>

__attribute__((constructor)) static void early(void) {
    fprintf(stderr, "gcd_test: constructor\n");
}

int main(void) {
    fprintf(stderr, "gcd_test: main started\n");
    
    dispatch_semaphore_t sem = dispatch_semaphore_create(0);
    if (!sem) {
        fprintf(stderr, "ERROR: dispatch_semaphore_create failed\n");
        return 1;
    }
    fprintf(stderr, "gcd_test: semaphore created\n");
    
    dispatch_release(sem);
    fprintf(stderr, "gcd_test: semaphore released\n");
    
    fprintf(stderr, "gcd_test: SUCCESS\n");
    return 0;
}
