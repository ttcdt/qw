/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#ifndef QW_SH_H
#define QW_SH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qw_char.h"


typedef struct qw_sh_ext qw_sh_ext_t;

struct qw_sh_ext {
    char *s;            /* extension */
    qw_sh_ext_t *n;
};

typedef struct qw_sh_tok qw_sh_tok_t;

struct qw_sh_tok {
    size_t z;           /* size of string */
    qw_attr_t a;        /* attribute */
    qw_sh_tok_t *n;
    qw_char_t s[1];
};


typedef struct qw_sh_blk qw_sh_blk_t;

struct qw_sh_blk {
    qw_char_t *b;       /* beginning of block */
    size_t bz;          /* size of b */
    qw_char_t *e;       /* end of block */
    size_t ez;          /* size of e */
    qw_char_t *m;       /* middle string */
    size_t mz;          /* size of m */
    qw_attr_t a;        /* attribute */
    qw_sh_blk_t *n;
    qw_char_t s[1];
};


typedef struct qw_sh qw_sh_t;

struct qw_sh {
    wchar_t *id;        /* identifier */
    qw_sh_ext_t *e;     /* extension list */
    qw_sh_tok_t *t;     /* token list */
    qw_sh_blk_t *b;     /* block list */
    qw_sh_t *n;
};

qw_sh_t *qw_sh_find(qw_sh_t *s, wchar_t *id);

qw_sh_t *qw_sh_new(wchar_t *id, qw_sh_t *s);

qw_sh_ext_t *qw_sh_new_ext(qw_sh_ext_t *n, char *s);

qw_sh_tok_t *qw_sh_new_token(qw_sh_tok_t *t, qw_char_t *s, qw_attr_t a);

qw_sh_blk_t *qw_sh_new_block(qw_sh_blk_t *t, qw_char_t *b, qw_char_t *e,
                             qw_char_t *m, qw_attr_t a);

void qw_sh_apply(qw_sh_t *sh, qw_char_t *s, off_t v, size_t z);

qw_sh_t *qw_sh_find_by_fn(qw_sh_t *sc, char *fn);

#ifdef __cplusplus
}
#endif

#endif /* QW_SH_H */
