/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#ifndef QW_EDIT_H
#define QW_EDIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qw_blk.h"
#include "qw_jrnl.h"
#include "qw_sh.h"
#include "qw_fb.h"
#include "qw_op.h"
#include "qw_event.h"

typedef struct qw_edit qw_edit_t;

struct qw_edit {
    char *fn;           /* file name */
    off_t ac;           /* absolute cursor */
    off_t ms;           /* mark start */
    off_t me;           /* mark end */
    qw_jrnl_t *j;       /* journal */
    qw_blk_t *b;        /* block */
    qw_sh_t *sh;        /* syntax highlight definitions */
    int m;              /* modification-before-saving counter */
    int cr;             /* write CRLF instead of LF */
};

extern int qw_tab_size;

qw_edit_t *qw_edit_new(char *fn, qw_sh_t *s);

void qw_edit_destroy(qw_edit_t *e);

void qw_edit_paint(qw_edit_t *e, off_t *v, qw_fb_t *fb);

void qw_edit_append(qw_edit_t *e, qw_char_t *str, size_t z);

qw_op_t qw_edit_op(qw_edit_t *e, qw_event_t *ev);

#ifdef __cplusplus
}
#endif

#endif /* QW_EDIT_H */
