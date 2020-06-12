/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "qw_sh.h"


qw_sh_t *qw_sh_find(qw_sh_t *s, wchar_t *id)
{
    if (s && wcscmp(s->id, id) != 0)
        s = qw_sh_find(s->n, id);

    return s;
}


qw_sh_t *qw_sh_new(wchar_t *id, qw_sh_t *s)
{
    qw_sh_t *sh;

    sh = calloc(sizeof(qw_sh_t), 1);

    sh->id  = wcsdup(id);
    sh->n   = s;

    return sh;
}


qw_sh_ext_t *qw_sh_new_ext(qw_sh_ext_t *n, char *s)
{
    qw_sh_ext_t *e;

    e = malloc(sizeof(*e));

    e->s = strdup(s);
    e->n = n;

    return e;
}


qw_sh_tok_t *qw_sh_new_token(qw_sh_tok_t *t, qw_char_t *s, qw_attr_t a)
{
    qw_sh_tok_t *n;
    size_t z;

    z = wcslen((wchar_t *)s);

    if (t && z > t->z)
        t->n = qw_sh_new_token(t->n, s, a);
    else {
        n = malloc(sizeof(qw_sh_tok_t) + z * sizeof(qw_char_t));
        n->z = z;
        n->a = a;
        n->n = t;
        memcpy(n->s, s, z * sizeof(qw_char_t));

        t = n;
    }

    return t;
}


qw_sh_blk_t *qw_sh_new_block(qw_sh_blk_t *t, qw_char_t *b, qw_char_t *e,
                             qw_char_t *m, qw_attr_t a)
{
    qw_sh_blk_t *n;

    if (t)
        t->n = qw_sh_new_block(t->n, b, e, m, a);
    else {
        size_t bz, ez, mz, z;

        bz = wcslen((wchar_t *)b);
        ez = e ? wcslen((wchar_t *)e) : 0;
        mz = m ? wcslen((wchar_t *)m) : 0;

        z = sizeof(qw_sh_blk_t) + (bz + ez + mz) * sizeof(qw_char_t);
        n = calloc(z, 1);

        n->a = a;

        n->bz = bz;
        n->ez = ez;
        n->mz = mz;

        n->b = n->s;
        n->e = n->b + bz;
        n->m = mz ? n->e + ez : NULL;

        memcpy(n->b, b, bz * sizeof(qw_char_t));
        if (n->e)
            memcpy(n->e, e, ez * sizeof(qw_char_t));
        if (n->m)
            memcpy(n->m, m, mz * sizeof(qw_char_t));

        t = n;
    }

    return t;
}


static void apply_tokens(qw_sh_tok_t *t, qw_char_t *s, off_t c, const size_t z)
{
    while (c < z) {
        off_t i;
        qw_sh_tok_t *n = t;
        size_t tz;

        while (c < z && !(iswalnum(s[c]) || s[c] == L'_'))
            c++;

        i = c;

        while (c < z && (iswalnum(s[c]) || s[c] == L'_'))
            c++;

        while (n) {
            off_t t = c - i;
            tz = n->z;

            if (n->s[0] == L'0' && s[i] >= L'0' && s[i] <= L'9') {
                tz = t;
                break;
            }
            else
            if (t < n->z)
                n = NULL;
            else
            if (t == n->z && memcmp(&s[i], n->s, n->z * sizeof(*s)) == 0)
                break;
            else
                n = n->n;
        }

        if (n)
            qw_char_set_attr(s, i, tz, n->a);
    }
}


static int find_word(qw_char_t *s, off_t c, size_t sz,
                     qw_char_t *w, size_t wz, off_t *f)
{
    int r = 0;

    while (w && !r && sz > wz && c < sz - wz) {
        if (memcmp(&s[c], w, wz * sizeof(qw_char_t)) == 0) {
            *f = c;
            r = 1;
        }

        c++;
    }

    return r;
}


static void apply_blocks(qw_sh_blk_t *b, qw_char_t *s, const size_t z)
{
    off_t c = 0;

    if (b) {
        while (find_word(s, c, z, b->b, b->bz, &c)) {
            off_t i = c + b->bz;

            while (b->e) {
                off_t m = z;
                off_t f = z;

                find_word(s, i, z, b->m, b->mz, &m);

                if (!find_word(s, i, z, b->e, b->ez, &f)) {
                    i = z;
                    break;
                }

                if (f < m) {
                    i = f + b->ez;
                    break;
                }

                i = m + b->mz;
            }

            qw_char_set_attr(s, c, i - c, b->a);
            c = i;
        }

        apply_blocks(b->n, s, z);
    }
}


void qw_sh_apply(qw_sh_t *sh, qw_char_t *s, off_t v, size_t z)
{
    apply_blocks(sh->b, s, z);
    apply_tokens(sh->t, s, v, z);
}


qw_sh_t *qw_sh_find_by_fn(qw_sh_t *sc, char *fn)
{
    int i = strlen(fn);

    while (sc) {
        qw_sh_ext_t *e = sc->e;

        while (e) {
            int t = strlen(e->s);

            if (i >= t && strcmp(e->s, fn + i - t) == 0)
                goto found;

            e = e->n;
        }

        sc = sc->n;
    }

found:
    return sc;
}

