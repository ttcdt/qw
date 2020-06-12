/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#ifndef QW_FB_H
#define QW_FB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qw_char.h"

typedef struct qw_fb qw_fb_t;

struct qw_fb {
    size_t w;           /* width */
    size_t h;           /* height */
    qw_char_t f;        /* filler char */
    qw_char_t l;        /* end-of-line char */
    qw_char_t u;        /* unprintable char */
    qw_char_t d[1];     /* data */
};

qw_fb_t *qw_fb_new(size_t w, size_t h);

qw_fb_t *qw_fb_destroy(qw_fb_t *f);

size_t qw_fb_get(qw_fb_t *f, off_t *x, off_t y, qw_attr_t *a, qw_char_t *s);

void qw_fb_put(qw_fb_t *f, qw_char_t *s, size_t z);

void qw_fb_mix(qw_fb_t *f, qw_fb_t *s, off_t x, off_t y);

size_t qw_fb_diff(qw_fb_t *f1, qw_fb_t *f2);

#ifdef __cplusplus
}
#endif

#endif /* QW_FB_H */
