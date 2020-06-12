/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "qw_jrnl.h"


qw_jrnl_t *qw_jrnl_destroy(qw_jrnl_t *j)
{
    if (j) {
        qw_jrnl_destroy(j->n);
        free(j);
    }

    return NULL;
}


qw_jrnl_t *qw_jrnl_new(int i, qw_blk_t *b, off_t c,
                         qw_char_t *s, size_t z, qw_jrnl_t *p)
{
    qw_jrnl_t *j;

    j = malloc(sizeof(qw_jrnl_t) + z * sizeof(qw_char_t));

    j->i = i;
    j->o = qw_blk_to_abs(b, c);
    j->z = z;
    j->p = p;
    j->n = NULL;

    if (i)
        memcpy(j->d, s, z * sizeof(qw_char_t));
    else
        qw_blk_get(b, c, j->d, z);

    if (p) {
        qw_jrnl_destroy(p->n);
        p->n = j;
    }

    return j;
}


qw_blk_t *qw_jrnl_apply(qw_blk_t *b, qw_jrnl_t *j, int d)
{
    off_t c = 0;

    b = qw_blk_to_rel(b, j->o, &c);

    if (j->i == d)
        b = qw_blk_insert(b, c, j->d, j->z);
    else
        qw_blk_delete(b, c, j->z);

    return b;
}
