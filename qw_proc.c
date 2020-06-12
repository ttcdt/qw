/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "qw_utf8.h"
#include "qw_proc.h"

#define KEY_QUEUE_SIZE 16

qw_proc_t *qw_proc_new(qw_proc_t *n, qw_edit_t *e,
                       qw_op_t *k, void *(*func)(void *))
{
    qw_proc_t *p;

    p = calloc(sizeof(qw_proc_t), 1);

    p->e = e;
    p->k = k;

    p->q = calloc(KEY_QUEUE_SIZE, sizeof(qw_event_t));

    qw_mutex_init(&p->m);
    qw_mutex_init(&p->qm);
    qw_sem_init(&p->qs);

    if (n) {
        p->n = n;
        p->p = n->p;

        p->n->p = p;
        p->p->n = p;
    }
    else {
        p->n = p;
        p->p = p;
    }

    qw_thread(&p->t, func, p);

    return p;
}


qw_proc_t *qw_proc_destroy(qw_proc_t *p)
{
    qw_proc_t *r = NULL;

    if (p->e)
        qw_edit_destroy(p->e);

    qw_mutex_destroy(&p->m);
    qw_mutex_destroy(&p->qm);
    free(p->q);

    p->p->n = p->n;
    p->n->p = p->p;

    if (p->n != p)
        r = p->n;

    free(p);

    return r;
}


void qw_proc_post_op(qw_proc_t *p, qw_event_t *ev)
{
    qw_mutex_lock(&p->qm);

    p->q[p->qh] = *ev;

    p->qh++;
    p->qh %= KEY_QUEUE_SIZE;

    if (p->qt == p->qh) {
        p->qt++;
        p->qt %= KEY_QUEUE_SIZE;
    }

    qw_mutex_unlock(&p->qm);

    qw_sem_post(&p->qs);
}


int qw_proc_wait_op(qw_proc_t *p, qw_event_t *ev, int blk)
{
    int r = -1;

    if ((r = qw_sem_wait(&p->qs, blk))) {
        *ev = p->q[p->qt];

        p->qt++;
        p->qt %= KEY_QUEUE_SIZE;
    }

    return r;
}


#define LOAD_BUF 1024

void *qw_proc_edit(void *v)
{
    qw_proc_t *p = (qw_proc_t *)v;
    FILE *f;

    if ((f = fopen(p->e->fn, "rb")))
        p->b++;

    for (;;) {
        qw_event_t ev;

        if (f) {
            qw_char_t str[LOAD_BUF];
            size_t z;

            if ((z = qw_utf8_read(f, str, LOAD_BUF, &p->e->cr))) {
                qw_mutex_lock(&p->m);
                qw_edit_append(p->e, str, z);
                qw_mutex_unlock(&p->m);
            }
            else {
                fclose(f);
                f = NULL;
                p->b--;
            }
        }

        if (qw_proc_wait_op(p, &ev, f == NULL)) {
            qw_mutex_lock(&p->m);
            qw_op_t o = qw_edit_op(p->e, &ev);
            qw_mutex_unlock(&p->m);

            if (o == QW_OP_DESTROY)
                break;
            else
            if (o != QW_OP_NOP) {
                ev.o = o;
                qw_proc_post_op(p, &ev);
            }
        }
    }

    qw_mutex_lock(&p->m);
    qw_edit_destroy(p->e);
    p->e = NULL;
    qw_mutex_unlock(&p->m);

    return NULL;
}

