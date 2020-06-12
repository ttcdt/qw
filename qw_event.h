/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#ifndef QW_EVENT_H
#define QW_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qw_op.h"
#include "qw_char.h"
#include "qw_fb.h"

typedef struct qw_event qw_event_t;

struct qw_event {
    qw_op_t o;
    qw_char_t w;
    qw_fb_t *f;
    void *c;        /* context (really qw_core_t *) */
};

#ifdef __cplusplus
}
#endif

#endif /* QW_EVENT_H */
