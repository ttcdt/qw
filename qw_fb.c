/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "qw_fb.h"


qw_fb_t *qw_fb_new(size_t w, size_t h)
{
    qw_fb_t *fb;

    fb = calloc(sizeof(qw_fb_t) + w * h * sizeof(qw_char_t), 1);

    fb->w = w;
    fb->h = h;
    fb->f = L' ';
    fb->l = L' ';
    fb->u = L'\xb7';

    return fb;
}


qw_fb_t *qw_fb_destroy(qw_fb_t *f)
{
    free(f);
    return NULL;
}


size_t qw_fb_get(qw_fb_t *f, off_t *x, off_t y, qw_attr_t *a, qw_char_t *s)
{
    off_t c = *x;
    size_t z = 0;
    qw_char_t *o = &f->d[y * f->w + c];

    while (c < f->w && !*o) {
        c++;
        o++;
    }

    if (c < f->w) {
        *x = c;

        *a = QW_GET_ATTR(*o);

        while (c < f->w && *o && QW_GET_ATTR(*o) == *a) {
            s[z++] = QW_WCHAR(*o);
            o++;
            c++;
        }
        s[z] = L'\0';
    }

    return z;
}


void qw_fb_put(qw_fb_t *f, qw_char_t *s, size_t z)
{
    int n, m;
    qw_char_t *o = f->d;

    for (n = 0; z > 0 && n < f->h; n++) {
        off_t c = qw_char_next_row(s, z, f->w);

        for (m = 0; m < c; m++) {
            qw_char_t w = *s++;

            if (QW_WCHAR(w) == L'\n')
                w = QW_SET_ATTR(f->l, QW_GET_ATTR(w));
            else
            if (QW_WCHAR(w) == L'\t')
                w = QW_SET_ATTR(L'\x2192', QW_GET_ATTR(w));
            else
            if (QW_WCHAR(w) < L' ')
                w = QW_SET_ATTR(f->u, QW_GET_ATTR(w));

            *o++ = w;
            z--;
        }

        while (c++ < f->w)
            *o++ = f->f;
    }

    for (; n < f->h; n++) {
        off_t c = 0;

        while (c++ < f->w)
            *o++ = f->f;
    }
}


void qw_fb_mix(qw_fb_t *f, qw_fb_t *s, off_t x, off_t y)
{
    int n;
    qw_char_t *i = s->d;
    qw_char_t *o = f->d + f->w * y + x;

    for (n = 0; n < s->h; n++) {
        int m;

        for (m = 0; m < s->w; m++)
            o[m] = *i++;

        o += f->w;
    }
}


size_t qw_fb_diff(qw_fb_t *f1, qw_fb_t *f2)
{
    int n;
    qw_char_t *p1 = f1->d;
    qw_char_t *p2 = f2->d;
    size_t c = 0;

    for (n = f1->h * f1->w; n > 0; n--) {
        if (*p1 == *p2)
            *p1 = 0;
        else {
            *p2 = *p1;
            c++;
        }

        p1++;
        p2++;
    }

    return c;
}
