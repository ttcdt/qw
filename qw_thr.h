/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#ifndef QW_THR_H
#define QW_THR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#ifdef CONFOPT_PTHREADS

#include <pthread.h>
#include <semaphore.h>

typedef pthread_t qw_thread_t;
typedef pthread_mutex_t qw_mutex_t;
typedef sem_t qw_sem_t;

#endif /* CONFOPT_PTHREADS */

#ifdef CONFOPT_WIN32

#include <windows.h>

typedef HANDLE qw_thread_t;
typedef HANDLE qw_mutex_t;
typedef HANDLE qw_sem_t;

#endif /* CONFOPT_WIN32 */

int qw_mutex_init(qw_mutex_t *m);

int qw_mutex_destroy(qw_mutex_t *m);

int qw_mutex_lock(qw_mutex_t *m);

int qw_mutex_unlock(qw_mutex_t *m);

int qw_thread(qw_thread_t *t, void *(*func)(void *), void *args);

void qw_thread_join(qw_thread_t t);

int qw_sem_init(qw_sem_t *s);

int qw_sem_post(qw_sem_t *s);

int qw_sem_wait(qw_sem_t *s, int blk);


#ifdef __cplusplus
}
#endif

#endif /* QW_THR_H */
