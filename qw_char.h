/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#ifndef QW_CHAR_H
#define QW_CHAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>

typedef wchar_t qw_char_t;

#ifdef CONFOPT_WIN32
#define CONFOPT_WCHAR_16
#else
#define CONFOPT_WCHAR_32
#endif

#ifdef CONFOPT_WCHAR_16
#define QW_CHAR_MASK        0x07ff
#define QW_SET_ATTR(c, a)   (QW_WCHAR(c) | (a << 11))
#define QW_GET_ATTR(c)      ((c) >> 11)
#else /* CONFOPT_WCHAR_16 */
#define QW_CHAR_MASK        0x07ffffff
#define QW_SET_ATTR(c, a)   (QW_WCHAR(c) | (a << 27))
#define QW_GET_ATTR(c)      ((c) >> 27)
#endif /* CONFOPT_WCHAR_16 */

#define QW_WCHAR(c)    ((c) & QW_CHAR_MASK)

typedef enum {
    QW_ATTR_NORMAL,
    QW_ATTR_CURSOR,
    QW_ATTR_MARK,
    QW_ATTR_MATCHING,
    QW_ATTR_WORD1,
    QW_ATTR_WORD2,
    QW_ATTR_WORD3,
    QW_ATTR_COMMENTS,
    QW_ATTR_LITERAL,
    QW_ATTR_DOCUMENTATION,
    QW_ATTR_TAG,
    QW_ATTR_SPELL,
    QW_ATTR_SEARCH,
    QW_ATTR_COUNT
} qw_attr_t;


off_t qw_char_next_row(const qw_char_t *s, size_t z, size_t w);

void qw_char_set_attr(qw_char_t *s, off_t c, size_t z, qw_attr_t a);

#ifdef __cplusplus
}
#endif

#endif /* QW_CHAR_H */
