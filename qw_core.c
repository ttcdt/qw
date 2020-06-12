/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "qw_core.h"

qw_core_t *qw_core_new(void)
{
    return calloc(sizeof(struct _qw_core), 1);
}


void qw_core_new_fb(qw_core_t *c, size_t w, size_t h)
{
    qw_fb_destroy(c->fb);
    c->fb = qw_fb_new(w, h);

    qw_fb_destroy(c->pfb);
    c->pfb = qw_fb_new(w, h);

/*    if (c->p) {
        qw_proc_t *p = c->p;

        do {
            p->e->fb = c->fb;
            p = p->n;
        } while (p != c->p);
    }*/
}


void qw_core_new_key(qw_core_t *c, qw_key_t k, qw_char_t w)
{
    if (c->p) {
        qw_event_t ev;

        ev.o = c->p->k[k];
        ev.w = w;
        ev.f = c->fb;
        ev.c = c;

        if (ev.o == QW_OP_QUIT) {
            qw_proc_t *p = c->p;

            ev.o = QW_OP_CLOSE;

            do {
                qw_proc_post_op(p, &ev);
                p = p->n;
            } while (p != c->p);
        }
        else
        if (ev.o == QW_OP_NEXT) {
            c->p = c->p->n;
        }
        else
            qw_proc_post_op(c->p, &ev);
    }
}


size_t qw_core_update(qw_core_t *c)
{
    size_t z = 0;

    if (c->p) {
        if (c->p->e) {
            qw_mutex_lock(&c->p->m);

            qw_edit_paint(c->p->e, &c->p->v, c->fb);

            if (c->p->b)
/*                c->fb->d[0] = L'\x2026';*/
                c->fb->d[0] = L"/-\\|"[((int)time(NULL))&0x3];

            qw_mutex_unlock(&c->p->m);

            z = qw_fb_diff(c->fb, c->pfb);
        }
        else
            c->p = qw_proc_destroy(c->p);
    }
    else
        c->rf = 0;

    return z;
}


void qw_core_open(qw_core_t *c, char *fn)
{
    qw_sh_t *s = qw_sh_find_by_fn(c->s, fn);
    qw_edit_t *e = qw_edit_new(fn, s);

    c->p = qw_proc_new(c->p, e, c->ke, qw_proc_edit);
}

