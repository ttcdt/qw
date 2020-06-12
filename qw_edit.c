/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "qw_edit.h"
#include "qw_utf8.h"
#include "qw_cf.h"


qw_edit_t *qw_edit_new(char *fn, qw_sh_t *s)
{
    qw_edit_t *e;

    e = calloc(sizeof(qw_edit_t), 1);

    /* FIXME: ugly */
    e->j = qw_jrnl_new(1, NULL, 0, L"", 0, NULL);

    e->ms = -1;
    e->me = -1;

    e->fn = strdup(fn);

    e->sh = s;

    return e;
}


void qw_edit_destroy(qw_edit_t *e)
{
    free(e->fn);
    qw_jrnl_destroy(e->j);
    free(e);
}


static void fix_av(qw_edit_t *e, off_t *v, qw_fb_t *fb)
{
    off_t c, i, l;
    qw_blk_t *b;

    if (e->ac < *v)
        *v = qw_blk_row_abs(e->b, e->ac, fb->w);
    else {
        if (*v + (fb->w * fb->h) < e->ac) {
            if ((*v = e->ac - (fb->w * fb->h)) <= 0)
                *v = 0;
        }

        b = qw_blk_to_rel(e->b, *v, &c);
        qw_blk_row(e->b, *v, e->ac, fb->w, &l);

        while (l-- >= fb->h) {
            i = qw_blk_row_next(b, c, fb->w);
            b = qw_blk_move(b, c, i, &c);
            *v += i;
        }
    }
}


static void color_match_p(qw_char_t *s, off_t f, off_t c, off_t t)
{
    qw_char_t w, m;
    int d = 0, l = 0;

    w = s[c];
    switch (w) {
    case L'{': m = L'}'; d = 1;  break;
    case L'}': m = L'{'; d = -1; break;
    case L'(': m = L')'; d = 1;  break;
    case L')': m = L'('; d = -1; break;
    case L'[': m = L']'; d = 1;  break;
    case L']': m = L'['; d = -1; break;
    }

    if (d) {
        off_t o = d == 1 ? t : f;

        while (c != o) {
            if (s[c] == w)
                l++;
            if (s[c] == m)
                l--;

            if (!l) {
                qw_char_set_attr(s, c, 1, QW_ATTR_MATCHING);
                break;
            }

            c += d;
        }
    }
}

void qw_edit_paint(qw_edit_t *e, off_t *v, qw_fb_t *fb)
{
    qw_blk_t *b;
    off_t c, o;
    qw_char_t *s;
    size_t z;

    fix_av(e, v, fb);

    o = e->sh && e->sh->b ? 0 : *v;

    z = 1 + (fb->w * fb->h) + *v - o;
    s = calloc(z, sizeof(qw_char_t));

    b = qw_blk_to_rel(e->b, o, &c);
    z = qw_blk_get(b, c, s, z);

    if (!qw_blk_move(b, c, z + 1, &c))
        s[z++] = L' ';

    if (e->sh)
        qw_sh_apply(e->sh, s, *v - o, z);

    color_match_p(s, *v - o, e->ac - o, z);

    if (e->ms != -1 && e->me != -1)
        qw_char_set_attr(s, e->ms - o, e->me - e->ms, QW_ATTR_MARK);

    qw_char_set_attr(s, e->ac - o, 1, QW_ATTR_CURSOR);

    qw_fb_put(fb, s + *v - o, z);

    free(s);
}


void qw_edit_append(qw_edit_t *e, qw_char_t *s, size_t z)
{
    e->b = qw_blk_last(e->b);
    e->b = qw_blk_insert(e->b, e->b ? e->b->l : 0, s, z);
}


#define FILE_BUF_LEN 1024

void op_save(qw_edit_t *e)
{
    qw_char_t str[FILE_BUF_LEN];
    size_t z;
    FILE *f;
    off_t i;
    qw_blk_t *b;

    if ((f = fopen(e->fn, "wb")) != NULL) {
        i = 0;
        b = qw_blk_first(e->b);
        while ((z = qw_blk_get(b, i, str, FILE_BUF_LEN))) {
            qw_utf8_write(f, str, z, e->cr);
            b = qw_blk_move(b, i, z, &i);
        }

        fclose(f);

        e->m = 0;
    }
}


static void op_left(qw_edit_t *e)
{
    if (e->ac)
        e->ac--;
}


static void op_right(qw_edit_t *e)
{
    qw_blk_t *b;
    off_t c;

    b = qw_blk_to_rel(e->b, e->ac, &c);
    if ((b = qw_blk_move(b, c, 1, &c)))
        e->ac = qw_blk_to_abs(b, c);
}


static int op_del_mark(qw_edit_t *e)
{
    int r = 0;

    if (e->ms != -1 && e->me != -1) {
        qw_blk_t *b;
        off_t c;
        size_t z;

        z = e->me - e->ms;
        e->ac = e->ms;

        b = qw_blk_to_rel(e->b, e->ac, &c);

        e->j = qw_jrnl_new(0, b, c, NULL, z, e->j);
        b = qw_jrnl_apply(b, e->j, 1);

        e->ms = e->me = -1;

        r = 1;

        e->m++;
    }

    return r;
}


static void op_char(qw_edit_t *e, qw_char_t w)
{
    qw_blk_t *b;
    off_t c = 0;

    op_del_mark(e);

    b = qw_blk_to_rel(e->b, e->ac, &c);

    e->j = qw_jrnl_new(1, b, c, &w, 1, e->j);
    b = qw_jrnl_apply(b, e->j, 1);

    e->b = qw_blk_move(b, c, 1, &c);
    e->ac = qw_blk_to_abs(b, c);

    e->m++;
}


static void op_del(qw_edit_t *e)
{
    qw_blk_t *b;
    off_t c;

    if (!op_del_mark(e)) {
        b = qw_blk_to_rel(e->b, e->ac, &c);

        e->j = qw_jrnl_new(0, b, c, NULL, 1, e->j);
        b = qw_jrnl_apply(b, e->j, 1);
    }

    e->m++;
}


static void op_backspace(qw_edit_t *e)
{
    if (e->ac > 0) {
        op_left(e);
        op_del(e);
    }
}


static void op_bof(qw_edit_t *e)
{
    e->ac = 0;
}


static void op_eof(qw_edit_t *e)
{
    e->b = qw_blk_last(e->b);
    e->ac = qw_blk_to_abs(e->b, e->b->l);
}


static void op_bol(qw_edit_t *e)
{
    qw_blk_t *b;
    off_t c;

    b = qw_blk_to_rel(e->b, e->ac, &c);

    e->b = qw_blk_move_bol(b, c, &c);
    e->ac = qw_blk_to_abs(e->b, c);
}


static void op_eol(qw_edit_t *e)
{
    qw_blk_t *b;
    off_t c;

    b = qw_blk_to_rel(e->b, e->ac, &c);

    e->b = qw_blk_move_eol(b, c, &c);
    e->ac = qw_blk_to_abs(e->b, c);
}


static void op_bor(qw_edit_t *e, qw_fb_t *fb)
{
    e->ac = qw_blk_row_abs(e->b, e->ac, fb->w);
}


static void op_eor(qw_edit_t *e, qw_fb_t *fb)
{
    qw_blk_t *b;
    off_t c;
    qw_char_t w;

    op_bor(e, fb);

    b = qw_blk_to_rel(e->b, e->ac, &c);

    if (qw_blk_get(b, c, &w, 1) && w != L'\n')
        e->ac += qw_blk_row_next(b, c, fb->w) - 1;
}


static void op_up(qw_edit_t *e, qw_fb_t *fb)
{
    qw_blk_t *b;
    off_t c;
    off_t i, j, d1, d2;

    i = qw_blk_row_abs(e->b, e->ac, fb->w);
    d1 = e->ac - i;

    if (i) {
        i--;
        j = qw_blk_row_abs(e->b, i, fb->w);
        d2 = i - j;

        b = qw_blk_to_rel(e->b, j, &c);
        b = qw_blk_move(b, c, d1 < d2 ? d1 : d2, &c);

        e->ac = qw_blk_to_abs(b, c);
    }
}


static void op_down(qw_edit_t *e, qw_fb_t *fb)
{
    qw_blk_t *b;
    off_t c = 0;
    off_t i, d1, d2;

    i = qw_blk_row_abs(e->b, e->ac, fb->w);
    d1 = e->ac - i;

    b = qw_blk_to_rel(e->b, i, &c);
    b = qw_blk_move(b, c, qw_blk_row_next(b, c, fb->w), &c);

    if ((d2 = qw_blk_row_next(b, c, fb->w)))
        d2--;

    b = qw_blk_move(b, c, d1 < d2 ? d1 : d2, &c);

    e->ac = qw_blk_to_abs(b, c);
}


static void op_pgup(qw_edit_t *e, qw_fb_t *fb)
{
    int n;

    for (n = 1; n < fb->h; n++)
        op_up(e, fb);
}


static void op_pgdn(qw_edit_t *e, qw_fb_t *fb)
{
    int n;

    for (n = 1; n < fb->h; n++)
        op_down(e, fb);
}


static void op_undo(qw_edit_t *e)
{
    /* FIXME: ugly */
    if (e->j->p) {
        e->b = qw_jrnl_apply(e->b, e->j, 0);
        e->ac = e->j->o;
        e->j = e->j->p;

        e->m++;
    }
}


static void op_tab(qw_edit_t *e, qw_fb_t *fb, qw_core_t *c)
{
    qw_blk_t *b;
    off_t i;
    size_t z;

    if (c->tz) {
        z = c->tz - ((e->ac - qw_blk_row_abs(e->b, e->ac, fb->w)) % c->tz);
        b = qw_blk_to_rel(e->b, e->ac, &i);

        e->j = qw_jrnl_new(1, b, i, L"        ", z, e->j);
        b = qw_jrnl_apply(b, e->j, 1);

        e->b = qw_blk_move(b, i, z, &i);
        e->ac = qw_blk_to_abs(b, i);

        e->m++;
    }
}


static void op_del_row(qw_edit_t *e, qw_fb_t *fb)
{
    qw_blk_t *b;
    off_t i, c;

    op_eor(e, fb);
    i = e->ac + 1;

    op_bor(e, fb);

    b = qw_blk_to_rel(e->b, e->ac, &c);

    e->j = qw_jrnl_new(0, b, c, NULL, i - e->ac, e->j);
    b = qw_jrnl_apply(b, e->j, 1);

    e->m++;
}


static void op_mark(qw_edit_t *e)
{
    if (e->ms == -1)
        e->ms = e->ac;
    else
    if (e->me == -1)
        e->me = e->ac;
    else
    if (e->ac < e->ms)
        e->ms = e->ac;
    else
    if (e->ac > e->me)
        e->me = e->ac;

    if (e->ms > e->me) {
        off_t t = e->ms;
        e->ms = e->me;
        e->me = t;
    }
}


static void op_unmark(qw_edit_t *e)
{
    e->ms = e->me = -1;
}


static void op_copy(qw_edit_t *e, qw_core_t *c)
{
    qw_blk_t *b;
    off_t i;

    b = qw_blk_to_rel(e->b, e->ms, &i);

    c->cz = e->me - e->ms;
    c->cd = realloc(c->cd, c->cz * sizeof(qw_char_t));

    qw_blk_get(b, i, c->cd, c->cz);

    if (c->copy)
        c->copy(c);
}


static void op_paste(qw_edit_t *e, qw_core_t *c)
{
    qw_blk_t *b;
    off_t i;

    if (c->paste)
        c->paste(c);

    op_del_mark(e);

    if (c->cd && c->cz) {
        b = qw_blk_to_rel(e->b, e->ac, &i);

        e->j = qw_jrnl_new(1, b, i, c->cd, c->cz, e->j);
        b = qw_jrnl_apply(b, e->j, 1);

        e->b = qw_blk_move(b, i, c->cz, &i);
        e->ac = qw_blk_to_abs(b, i);
    }

    e->m++;
}


static void op_cut(qw_edit_t *e, qw_core_t *c)
{
    if (e->ms != -1 && e->me != -1) {
        op_copy(e, c);
        op_del_mark(e);
    }
}


static void op_search(qw_edit_t *e, qw_core_t *c)
{
    if (c->sd && c->sz) {
        qw_blk_t *b;
        off_t i;

        b = qw_blk_to_rel(e->b, e->ac, &i);
        if ((b = qw_blk_search(b, i, &i, c->sd, c->sz, 1))) {
            e->ac = qw_blk_to_abs(b, i);
        }
    }
}


qw_op_t qw_edit_op(qw_edit_t *e, qw_event_t *ev)
{
    qw_op_t r = QW_OP_NOP;
    qw_core_t *c = ev->c;

    switch (ev->o) {
    case QW_OP_LEFT:
        op_left(e);
        break;

    case QW_OP_RIGHT:
        op_right(e);
        break;

    case QW_OP_UP:
        op_up(e, ev->f);
        break;

    case QW_OP_DOWN:
        op_down(e, ev->f);
        break;

    case QW_OP_PGUP:
        op_pgup(e, ev->f);
        break;

    case QW_OP_PGDN:
        op_pgdn(e, ev->f);
        break;

    case QW_OP_BOL:
        op_bol(e);
        break;

    case QW_OP_EOL:
        op_eol(e);
        break;

    case QW_OP_BOR:
        op_bor(e, ev->f);
        break;

    case QW_OP_EOR:
        op_eor(e, ev->f);
        break;

    case QW_OP_BOF:
        op_bof(e);
        break;

    case QW_OP_EOF:
        op_eof(e);
        break;

    case QW_OP_CHAR:
        op_char(e, ev->w);
        break;

    case QW_OP_DEL:
        op_del(e);
        break;

    case QW_OP_BACKSPACE:
        op_backspace(e);
        break;

    case QW_OP_NEWLINE:
        op_char(e, L'\n');
        break;

    case QW_OP_TAB:
        op_tab(e, ev->f, c);
        break;

    case QW_OP_HARD_TAB:
        op_char(e, L'\t');
        break;

    case QW_OP_DEL_ROW:
        op_del_row(e, ev->f);
        break;

    case QW_OP_UNDO:
        op_undo(e);
        break;

    case QW_OP_MARK:
        op_mark(e);
        break;

    case QW_OP_UNMARK:
        op_unmark(e);
        break;

    case QW_OP_COPY:
        op_copy(e, c);
        op_unmark(e);

        break;

    case QW_OP_PASTE:
        op_paste(e, c);
        break;

    case QW_OP_CUT:
        op_cut(e, c);
        break;

    case QW_OP_SAVE:
        op_save(e);
        break;

    case QW_OP_SHOW_CODES:
        ev->f->l = L" \xb6"[!!(ev->f->l == L' ')];
        break;

    case QW_OP_SEARCH:
        op_search(e, c);
        break;

    case QW_OP_CLOSE:
        r = QW_OP_DESTROY;
        break;

    case QW_OP_DESTROY:
        r = QW_OP_DESTROY;
        break;

    case QW_OP_M_DASH:
        op_char(e, L'\x2014');
        break;

    case QW_OP_NOP:
    case QW_OP_COUNT:
        break;
    }

    return r;
}

