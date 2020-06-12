/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#ifndef QW_UTF8_H
#define QW_UTF8_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qw_char.h"

size_t qw_utf8_read(FILE *f, qw_char_t *p, size_t z, int *cr);

void qw_utf8_write(FILE *f, const qw_char_t *p, size_t z, int cr);

int qw_utf8_wc_to_mb(const qw_char_t cw, char *mb, int cr);

#ifdef __cplusplus
}
#endif

#endif /* QW_UTF8_H */
