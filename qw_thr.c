/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#include <stdlib.h>

#include "qw_thr.h"

#ifdef CONFOPT_PTHREADS

int qw_mutex_init(qw_mutex_t *m)
{
    return pthread_mutex_init(m, NULL);
}


int qw_mutex_destroy(qw_mutex_t *m)
{
    return pthread_mutex_destroy(m);
}


int qw_mutex_lock(qw_mutex_t *m)
{
    return pthread_mutex_lock(m);
}


int qw_mutex_unlock(qw_mutex_t *m)
{
    return pthread_mutex_unlock(m);
}


int qw_thread(qw_thread_t *t, void *(*func)(void *), void *args)
{
    return pthread_create(t, NULL, func, args);
}


void qw_thread_join(qw_thread_t t)
{
    pthread_join(t, NULL);
}


int qw_sem_init(qw_sem_t *s)
{
    return sem_init(s, 0, 0);
}


int qw_sem_post(qw_sem_t *s)
{
    return sem_post(s);
}


int qw_sem_wait(qw_sem_t *s, int blk)
{
    return !((blk ? sem_wait(s) : sem_trywait(s)) == -1);
}


#endif /* CONFOPT_PTHREADS */

#ifdef CONFOPT_WIN32

int qw_mutex_init(qw_mutex_t *m)
{
    *m = CreateMutex(NULL, FALSE, NULL);

    return !!*m;
}


int qw_mutex_destroy(qw_mutex_t *m)
{
    return 0;
}


int qw_mutex_lock(qw_mutex_t *m)
{
    return WaitForSingleObject(*m, INFINITE);
}


int qw_mutex_unlock(qw_mutex_t *m)
{
    return ReleaseMutex(*m);
}


int qw_thread(qw_thread_t *t, void *(*func)(void *), void *args)
{
    *t = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) func, args, 0, NULL);

    return *t != NULL;
}


void qw_thread_join(qw_thread_t t)
{
    WaitForSingleObject(t, INFINITE);
}


int qw_sem_init(qw_sem_t *s)
{
    *s = CreateSemaphore(NULL, 0, 0x7fffffff, NULL);

    return *s != NULL;
}


int qw_sem_post(qw_sem_t *s)
{
    return ReleaseSemaphore(*s, 1, NULL);
}


int qw_sem_wait(qw_sem_t *s, int blk)
{
    return !!(WaitForSingleObject(*s, blk ? INFINITE : 0) == WAIT_OBJECT_0);
}


#endif /* CONFOPT_WIN32 */
