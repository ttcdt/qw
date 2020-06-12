/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#ifndef QW_BLK_H
#define QW_BLK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qw_char.h"

typedef struct qw_blk qw_blk_t;

struct qw_blk {
    size_t z;           /* size of block */
    off_t l;            /* offset to last char + 1*/
    qw_blk_t *p;       /* previous */
    qw_blk_t *n;       /* next */
    qw_char_t d[1];    /* data */
};

extern int qw_blk_gran;

qw_blk_t *qw_blk_new(qw_blk_t *b, size_t z, off_t l,
                       qw_blk_t *p, qw_blk_t *n);

qw_blk_t *qw_blk_destroy(qw_blk_t *b);

qw_blk_t *qw_blk_insert(qw_blk_t *b, off_t c,
                          const qw_char_t *d, size_t z);

void qw_blk_delete(qw_blk_t *b, off_t c, size_t z);

qw_blk_t *qw_blk_first(qw_blk_t *b);

qw_blk_t *qw_blk_last(qw_blk_t *b);

size_t qw_blk_get(qw_blk_t *b, off_t c, qw_char_t *d, size_t z);

qw_blk_t *qw_blk_move(qw_blk_t *b, off_t c, off_t i, off_t *f);

off_t qw_blk_to_abs(qw_blk_t *b, off_t c);

qw_blk_t *qw_blk_to_rel(qw_blk_t *b, off_t a, off_t *f);

qw_blk_t *qw_blk_here(qw_blk_t *b, off_t c, off_t *f,
                        const qw_char_t *s, size_t z);

qw_blk_t *qw_blk_search(qw_blk_t *b, off_t c, off_t *f,
                          const qw_char_t *s, size_t z, off_t i);

qw_blk_t *qw_blk_move_bol(qw_blk_t *b, off_t c, off_t *f);

qw_blk_t *qw_blk_move_eol(qw_blk_t *b, off_t c, off_t *f);

off_t qw_blk_row_next(qw_blk_t *b, off_t c, size_t w);

off_t qw_blk_row(qw_blk_t *b, off_t af, off_t at, size_t w, off_t *l);

off_t qw_blk_row_abs(qw_blk_t *b, off_t a, size_t w);

off_t qw_blk_i_and_not_e(qw_blk_t *b, off_t a, off_t d, wchar_t *i, wchar_t *e);

#ifdef __cplusplus
}
#endif

#endif /* QW_BLK_H */
