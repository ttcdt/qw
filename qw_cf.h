/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#ifndef QW_CF_H
#define QW_CF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qw_core.h"

int qw_cf_cmd(qw_core_t *c, wchar_t *l);

#ifdef __cplusplus
}
#endif

#endif /* QW_CF_H */
