/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#ifndef QW_PROC_H
#define QW_PROC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qw_char.h"
#include "qw_thr.h"
#include "qw_op.h"
#include "qw_edit.h"
#include "qw_event.h"

typedef struct qw_proc qw_proc_t;

struct qw_proc {
    off_t v;            /* first visible char */
    qw_edit_t *e;       /* edit */
    qw_mutex_t m;       /* mutex for e */
    qw_sem_t qs;        /* queue semaphore */
    qw_mutex_t qm;      /* queue mutex */
    qw_event_t *q;      /* event queue */
    int qh;             /* queue head */
    int qt;             /* queue tail */
    qw_op_t *k;         /* key to op table */
    int b;              /* busy flag */
    qw_thread_t t;      /* thread handler */
    qw_proc_t *p;
    qw_proc_t *n;
};

qw_proc_t *qw_proc_new(qw_proc_t *n, qw_edit_t *e,
                       qw_op_t *k, void *(*func)(void *));
qw_proc_t *qw_proc_destroy(qw_proc_t *p);
void qw_proc_post_op(qw_proc_t *p, qw_event_t *ev);
int qw_proc_wait_op(qw_proc_t *p, qw_event_t *ev, int blk);

void *qw_proc_edit(void *v);

#ifdef __cplusplus
}
#endif

#endif /* QW_PROC_H */
