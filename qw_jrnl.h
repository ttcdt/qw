/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#ifndef QW_JRNL_H
#define QW_JRNL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qw_char.h"
#include "qw_blk.h"

typedef struct qw_jrnl qw_jrnl_t;

struct qw_jrnl {
    int i;              /* insert, 1; delete, 0 */
    off_t o;            /* operation offset */
    size_t z;           /* size of data */
    qw_jrnl_t *p;      /* previous */
    qw_jrnl_t *n;      /* next */
    qw_char_t d[1];    /* data */
};

qw_jrnl_t *qw_jrnl_new(int i, qw_blk_t *b, off_t c,
                         qw_char_t *s, size_t z, qw_jrnl_t *p);

qw_blk_t *qw_jrnl_apply(qw_blk_t *b, qw_jrnl_t *j, int d);

qw_jrnl_t *qw_jrnl_destroy(qw_jrnl_t *j);

#ifdef __cplusplus
}
#endif

#endif /* QW_JRNL_H */
