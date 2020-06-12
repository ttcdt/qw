/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#ifndef QW_CORE_H
#define QW_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qw_op.h"
#include "qw_key.h"
#include "qw_proc.h"
#include "qw_sh.h"
#include "qw_event.h"

typedef struct _qw_core qw_core_t;

struct _qw_core {
    qw_op_t ke[QW_KEY_COUNT];           /* editor key2op table */
    qw_proc_t *p;                       /* list of procs */
    qw_sh_t *s;                         /* syntax highlight definitions */
    int rf;                             /* running flag */
    qw_fb_t *fb;                        /* framebuffer */
    qw_fb_t *pfb;                       /* previous framebuffer */
    wchar_t *di;                        /* driver identifier */
    qw_char_t *cd;                      /* clipboard data */
    size_t cz;                          /* clipboard size */
    qw_char_t *sd;                      /* search string data */
    size_t sz;                          /* search string size */
    size_t tz;                          /* tab size */
    void (*exec)(qw_core_t *c);         /* driver main loop */
    void (*copy)(qw_core_t *c);         /* copy to drv clipboard */
    void (*paste)(qw_core_t *c);        /* paste from drv clipboard */
    void (*cf_color)(qw_core_t *c,
                  qw_attr_t t, int irgb,
                  int prgb, int rev);   /* driver define color */
};

qw_core_t *qw_core_new(void);
void qw_core_new_fb(qw_core_t *c, size_t w, size_t h);
void qw_core_new_key(qw_core_t *c, qw_key_t k, qw_char_t wc);
size_t qw_core_update(qw_core_t *c);
void qw_core_open(qw_core_t *c, char *fn);

#ifdef __cplusplus
}
#endif

#endif /* QW_CORE_H */
