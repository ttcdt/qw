/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "qw_cf.h"

struct _qw_cf *qw_cf = NULL;


#define MAX_CF_TOKEN_SIZE 1024

static wchar_t *cf_token(wchar_t *c, int *i)
{
    int n, o;
    wchar_t *t = NULL;
    wchar_t w;

    n = *i;

    while (c[n] == L' ')
        n++;

    t = calloc(MAX_CF_TOKEN_SIZE, sizeof(wchar_t));

    o = 0;

    while (o < MAX_CF_TOKEN_SIZE && (w = c[n]) != L' ' && w != L'\0') {
        if (w == L'\\') {
            w = c[++n];

            if (w == L'n')
                w = L'\n';
        }

        t[o++] = w;
        n++;
    }

    if (o == 0 || o == MAX_CF_TOKEN_SIZE) {
        free(t);
        t = NULL;
    }

    *i = n;

    return t;
}


static qw_sh_t *cf_find_or_create(qw_core_t *c, wchar_t *sh_id)
{
    qw_sh_t *ns;

    if ((ns = qw_sh_find(c->s, sh_id)) == NULL)
        ns = c->s = qw_sh_new(sh_id, c->s);

    return ns;
}


static int cf_sh_token(qw_core_t *c, wchar_t *l, int *i, qw_attr_t a)
{
    int r = 0;
    wchar_t *sh_id;

    if ((sh_id = cf_token(l, i))) {
        wchar_t *t;
        qw_sh_t *ns = cf_find_or_create(c, sh_id);

        while ((t = cf_token(l, i))) {
            ns->t = qw_sh_new_token(ns->t, t, a);
            free(t);
        }

        free(sh_id);
    }

    return r;
}


static int cf_sh_ext(qw_core_t *c, wchar_t *l, int *i)
{
    int r = 0;
    wchar_t *sh_id;

    if ((sh_id = cf_token(l, i))) {
        wchar_t *t;
        int n;
        qw_sh_t *s = cf_find_or_create(c, sh_id);

        while ((t = cf_token(l, i))) {
            char *mb = calloc(wcslen(t) + 1, 1);

            /* FIXME: crappy wc to mb conversion */
            for (n = 0; t[n]; n++)
                mb[n] = (char) t[n];

            s->e = qw_sh_new_ext(s->e, mb);

            free(mb);
            free(t);
        }

        free(sh_id);
    }

    return r;
}


static int cf_sh_block(qw_core_t *c, wchar_t *l, int *i, qw_attr_t a)
{
    int r = 0;
    wchar_t *sh_id;

    if ((sh_id = cf_token(l, i))) {
        wchar_t *t1, *t2, *t3;
        qw_sh_t *s = cf_find_or_create(c, sh_id);

        t1 = cf_token(l, i);
        t2 = cf_token(l, i);
        t3 = cf_token(l, i);

        s->b = qw_sh_new_block(s->b, t1, t2, t3, a);

        free(t3);
        free(t2);
        free(t1);
        free(sh_id);
    }

    return r;
}


static int cf_sh_word(qw_core_t *c, wchar_t *l, int *i, qw_attr_t a)
{
    int r = 0;
    wchar_t *sh_id;

    if ((sh_id = cf_token(l, i))) {
        wchar_t *t;
        qw_sh_t *ns = cf_find_or_create(c, sh_id);

        while ((t = cf_token(l, i))) {
            ns->b = qw_sh_new_block(ns->b, t, NULL, NULL, a);
            free(t);
        }

        free(sh_id);
    }

    return r;
}


static int cf_color(qw_core_t *c, wchar_t *l, int *i, qw_attr_t a)
{
    wchar_t *t;
    int irgb = -1;
    int prgb = -1;
    int rev = 0;
    int r = 0;

    if ((t = cf_token(l, i)) != NULL) {
        if (wcscmp(t, L"default") == 0)
            irgb = -1;
        else
            swscanf(t, L"%x", &irgb);

        free(t);

        if ((t = cf_token(l, i)) != NULL) {
            if (wcscmp(t, L"default") == 0)
                prgb = -1;
            else
                swscanf(t, L"%x", &prgb);

            free(t);

            if ((t = cf_token(l, i)) != NULL) {
                if (wcscmp(t, L"reverse") == 0)
                    rev = 1;

                free(t);
            }

            c->cf_color(c, a, irgb, prgb, rev);
        }
    }

    return r;
}

static struct {
    wchar_t *id;
    qw_key_t k;
} cf_k_id[] = {
#include "qw_cf_k.h"
    { NULL,         QW_KEY_NONE }
};

static struct {
    wchar_t *id;
    qw_op_t o;
} cf_o_id[] = {
#include "qw_cf_o.h"
    { NULL,         QW_OP_NOP }
};

static int cf_key(wchar_t *l, int *i, qw_op_t *kt)
{
    int r = 0;
    wchar_t *k_id;

    if ((k_id = cf_token(l, i))) {
        wchar_t *o_id;

        if ((o_id = cf_token(l, i))) {
            qw_key_t k = QW_KEY_NONE;
            qw_op_t o = QW_OP_NOP;
            int n;

            for (n = 0; cf_k_id[n].id; n++) {
                if (wcscmp(cf_k_id[n].id, k_id) == 0) {
                    k = cf_k_id[n].k;
                    break;
                }
            }

            for (n = 0; cf_o_id[n].id; n++) {
                if (wcscmp(cf_o_id[n].id, o_id) == 0) {
                    o = cf_o_id[n].o;
                    break;
                }
            }

            if (k != QW_KEY_NONE && o != QW_OP_NOP) {
                kt[k] = o;
                r = 1;
            }
        }

        free(o_id);
    }

    free(k_id);

    return r;
}


static int cf_tab_size(qw_core_t *c, wchar_t *l, int *i)
{
    int r = 0;
    wchar_t *ts;

    if ((ts = cf_token(l, i))) {
        r = swscanf(ts, L"%d", &c->tz);

        if (c->tz < 2 || c->tz > 8) {
            r = 0;
            c->tz = 4;
        }
    }

    free(ts);

    return r;
}


int qw_cf_cmd(qw_core_t *c, wchar_t *l)
{
    wchar_t *t;
    int r = 0;
    int n = 0;

    t = cf_token(l, &n);

    if (!t || t[0] == L'#') {
        r = 1;
    }
    else
    if (wcscmp(t, L"sh_token_word1") == 0)
        r = cf_sh_token(c, l, &n, QW_ATTR_WORD1);
    else
    if (wcscmp(t, L"sh_token_word2") == 0)
        r = cf_sh_token(c, l, &n, QW_ATTR_WORD2);
    else
    if (wcscmp(t, L"sh_token_word3") == 0)
        r = cf_sh_token(c, l, &n, QW_ATTR_WORD3);
    else
    if (wcscmp(t, L"sh_token_literal") == 0)
        r = cf_sh_token(c, l, &n, QW_ATTR_LITERAL);
    else
    if (wcscmp(t, L"sh_block_doc") == 0)
        r = cf_sh_block(c, l, &n, QW_ATTR_DOCUMENTATION);
    else
    if (wcscmp(t, L"sh_block_literal") == 0)
        r = cf_sh_block(c, l, &n, QW_ATTR_LITERAL);
    else
    if (wcscmp(t, L"sh_block_comment") == 0)
        r = cf_sh_block(c, l, &n, QW_ATTR_COMMENTS);
    else
    if (wcscmp(t, L"sh_word_word1") == 0)
        r = cf_sh_word(c, l, &n, QW_ATTR_WORD1);
    else
    if (wcscmp(t, L"sh_word_word2") == 0)
        r = cf_sh_word(c, l, &n, QW_ATTR_WORD2);
    else
    if (wcscmp(t, L"sh_word_word3") == 0)
        r = cf_sh_word(c, l, &n, QW_ATTR_WORD3);
    else
    if (wcscmp(t, L"sh_ext") == 0)
        r = cf_sh_ext(c, l, &n);
    else
    if (wcscmp(t, L"color_normal") == 0)
        r = cf_color(c, l, &n, QW_ATTR_NORMAL);
    else
    if (wcscmp(t, L"color_cursor") == 0)
        r = cf_color(c, l, &n, QW_ATTR_CURSOR);
    else
    if (wcscmp(t, L"color_mark") == 0)
        r = cf_color(c, l, &n, QW_ATTR_MARK);
    else
    if (wcscmp(t, L"color_matching") == 0)
        r = cf_color(c, l, &n, QW_ATTR_MATCHING);
    else
    if (wcscmp(t, L"color_word1") == 0)
        r = cf_color(c, l, &n, QW_ATTR_WORD1);
    else
    if (wcscmp(t, L"color_word2") == 0)
        r = cf_color(c, l, &n, QW_ATTR_WORD2);
    else
    if (wcscmp(t, L"color_word3") == 0)
        r = cf_color(c, l, &n, QW_ATTR_WORD3);
    else
    if (wcscmp(t, L"color_comment") == 0)
        r = cf_color(c, l, &n, QW_ATTR_COMMENTS);
    else
    if (wcscmp(t, L"color_literal") == 0)
        r = cf_color(c, l, &n, QW_ATTR_LITERAL);
    else
    if (wcscmp(t, L"color_doc") == 0)
        r = cf_color(c, l, &n, QW_ATTR_DOCUMENTATION);
    else
    if (wcscmp(t, L"key") == 0)
        r = cf_key(l, &n, c->ke);
    else
    if (wcscmp(t, L"tab_size") == 0)
        r = cf_tab_size(c, l, &n);

    free(t);

    return r;
}
