/* Copyright (c) 2017 Kristján Valur Jónsson */

#ifndef _FASTCOND_PATCH_H_
#define _FASTCOND_PATCH_H_

/*
 * Now, handy macros to patch your programs to override the
 * regular condition variable stuff
 */
#if defined FASTCOND_PATCH_WCOND
/* use the weak condition variable.  This is sufficient for simple
 * synchronization, but makes no promises about _which_ threads get awoken.
 */
#define pthread_cond_t fastcond_wcond_t
#define pthread_cond_init fastcond_wcond_init
#define pthread_cond_fini fastcond_wcond_fini
#define pthread_cond_wait fastcond_wcond_wait
#define pthread_cond_timedwait fastcond_wcond_timedwait
#define pthread_cond_signal fastcond_wcond_signal
#define pthread_cond_broadcast fastcond_wcond_broadcast
#undef PTHREAD_COND_INITIALIZER
#define PTHREAD_COND_INITIALIZER not_implemented

#elif defined FASTCOND_PATCH_COND
/* use the _strong_ condition variable, the one that only wakes up those
 * threads already waiting
 */
#define pthread_cond_t fastcond_cond_t
#define pthread_cond_init fastcond_cond_init
#define pthread_cond_fini fastcond_cond_fini
#define pthread_cond_wait fastcond_cond_wait
#define pthread_cond_timedwait fastcond_cond_timedwait
#define pthread_cond_signal fastcond_cond_signal
#define pthread_cond_broadcast fastcond_cond_broadcast
#undef PTHREAD_COND_INITIALIZER
#define PTHREAD_COND_INITIALIZER not_implemented

#endif

#endif /* ! defined _FASTCOND_PATCH_H_ */
