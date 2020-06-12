/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "qw_blk.h"

int qw_blk_gran = 32;


qw_blk_t *qw_blk_new(qw_blk_t *b, size_t z, off_t l,
                       qw_blk_t *p, qw_blk_t *n)
{
    z = (z / qw_blk_gran + 1) * qw_blk_gran;

    b = realloc(b, sizeof(qw_blk_t) + z * sizeof(qw_char_t));

    b->z = z;
    b->l = l;
    b->p = p;
    b->n = n;

    if (p)
        p->n = b;
    if (n)
        n->p = b;

    return b;
}


qw_blk_t *qw_blk_destroy(qw_blk_t *b)
{
    if (b) {
        qw_blk_destroy(b->n);
        free(b);
    }

    return NULL;
}


qw_blk_t *qw_blk_insert(qw_blk_t *b, off_t c,
                          const qw_char_t *d, size_t z)
{
    off_t r;

    if (b == NULL)
        b = qw_blk_new(NULL, 0, 0, NULL, NULL);

    r = b->l - c;

    if (r == 0) {
        if (b->z - b->l < z)
            b = qw_blk_new(b, b->z + z, b->l, b->p, b->n);

        memcpy(&b->d[c], d, z * sizeof(qw_char_t));

        b->l += z;
    }
    else
    if (c == 0) {
        if (b->p)
            b = qw_blk_insert(b->p, b->p->l, d, z);
        else
            b = qw_blk_insert(qw_blk_new(NULL, z, 0, NULL, b), 0, d, z);
    }
    else {
        qw_blk_insert(qw_blk_new(NULL, r, 0, b, b->n), 0, &b->d[c], r);

        b->l = c;

        b = qw_blk_insert(b, c, d, z);
    }

    return b;
}


void qw_blk_delete(qw_blk_t *b, off_t c, size_t z)
{
    if (b && z > 0) {
        off_t r = b->l - c;

        if (r > z) {
            int n;

            qw_char_t *p = &b->d[c];

            for (n = r - z; n; n--, p++)
                *p = *(p + z);

            b->l = c + r - z;
        }
        else {
            b->l = c;
            qw_blk_delete(b->n, 0, z - r);
        }
    }
}


qw_blk_t *qw_blk_first(qw_blk_t *b)
{
    return b && b->p ? qw_blk_first(b->p) : b;
}


qw_blk_t *qw_blk_last(qw_blk_t *b)
{
    return b && b->n ? qw_blk_last(b->n) : b;
}


size_t qw_blk_get(qw_blk_t *b, off_t c, qw_char_t *d, size_t z)
{
    size_t r = 0;

    if (b && z) {
        r = b->l - c;

        if (r > z)
            r = z;

        memcpy(d, &b->d[c], r * sizeof(qw_char_t));

        r += qw_blk_get(b->n, 0, d + r, z - r);
    }

    return r;
}


qw_blk_t *qw_blk_move(qw_blk_t *b, off_t c, off_t i, off_t *f)
{
    if (b) {
        off_t r = (c == -1 ? b->l : c) + i;

        if (r >= 0 && r <= b->l)
            *f = r;
        else
        if (i < 0)
            b = qw_blk_move(b->p, -1, r, f);
        else
            b = qw_blk_move(b->n, 0, r - b->l, f);
    }

    return b;
}


off_t qw_blk_to_abs(qw_blk_t *b, off_t c)
{
    return b && b->p ? c + qw_blk_to_abs(b->p, b->p->l) : c;
}


qw_blk_t *qw_blk_to_rel(qw_blk_t *b, off_t a, off_t *f)
{
    return qw_blk_move(qw_blk_first(b), 0, a, f);
}


qw_blk_t *qw_blk_here(qw_blk_t *b, off_t c, off_t *f,
                        const qw_char_t *s, size_t z)
{
    qw_blk_t *r = NULL;

    if (b && z) {
        if (c >= b->l)
            r = qw_blk_here(b->n, 0, f, s, z);
        else
        if (b->d[c] == *s)
            r = qw_blk_here(b, c + 1, f, s + 1, z - 1);
    }
    else {
        r  = b;
        *f = c;
    }

    return r;
}


qw_blk_t *qw_blk_search(qw_blk_t *b, off_t c, off_t *f,
                          const qw_char_t *s, size_t z, off_t i)
{
    qw_blk_t *r = NULL;

    while (b && !(r = qw_blk_here(b, c, f, s, z)))
        b = qw_blk_move(b, c, i, &c);

    return r;
}


qw_blk_t *qw_blk_move_bol(qw_blk_t *b, off_t c, off_t *f)
{
    wchar_t w;
    qw_blk_t *r = b;

    if (qw_blk_get(r, c, &w, 1) && w == L'\n')
        r = qw_blk_move(r, c, -1, &c);

    if (!(r = qw_blk_search(r, c, f, L"\n", 1, -1))) {
        r = qw_blk_first(b);
        *f = 0;
    }

    return r;
}


qw_blk_t *qw_blk_move_eol(qw_blk_t *b, off_t c, off_t *f)
{
    wchar_t w;
    qw_blk_t *r = b;

    if (qw_blk_get(r, c, &w, 1) && w != L'\n') {
        if (!(r = qw_blk_search(r, c, f, L"\n", 1, 1))) {
            r = qw_blk_last(b);
            *f = r->l;
        }
        else
            r = qw_blk_move(r, *f, -1, f);
    }

    return r;
}


off_t qw_blk_row_next(qw_blk_t *b, off_t c, size_t w)
{
    qw_char_t s[1024];
    size_t z;

    z = qw_blk_get(b, c, s, w + 1);
    return qw_char_next_row(s, z, w);
}


off_t qw_blk_row(qw_blk_t *b, off_t af, off_t at, size_t w, off_t *l)
{
    off_t c, t, i, n;

    b = qw_blk_to_rel(b, af, &c);

    for (n = t = 0; ; n++, t += i) {
        i = qw_blk_row_next(b, c, w);

        if (!i || af + t + i > at)
            break;

        b = qw_blk_move(b, c, i, &c);
    }

    if (l)
        *l = n;

    return t;
}


off_t qw_blk_row_abs(qw_blk_t *b, off_t a, size_t w)
{
    off_t c, i;

    b = qw_blk_to_rel(b, a, &c);

    if ((b = qw_blk_move_bol(b, c, &c))) {
        i = qw_blk_to_abs(b, c);
        i += qw_blk_row(b, i, a, w, NULL);
    }
    else
        i = 0;

    return i;
}


#define I_AND_NOT_E(w, i, e) ((!i || wcschr(i, w)) && (!e || !wcschr(e, w)))

off_t qw_blk_i_and_not_e(qw_blk_t *b, off_t a, off_t d, wchar_t *i, wchar_t *e)
{
    qw_char_t w;
    qw_blk_t *r = b;
    off_t c, cc;

    r = qw_blk_to_rel(b, a, &c);
    cc = c;

    while (qw_blk_get(r, c, &w, 1) && I_AND_NOT_E(w, i, e)) {
        b = r;
        cc = c;
        r = qw_blk_move(r, c, d, &c);
    }

    if (r)
        b = r;

    return qw_blk_to_abs(b, cc);
}

