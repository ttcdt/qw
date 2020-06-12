/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#include "qw_char.h"


off_t qw_char_next_row(const qw_char_t *s, size_t z, size_t w)
{
    off_t c = 0;
    size_t t = 0;

    if (z < w)
        w = z;

    while (c < w) {
        t = 0;

        if (QW_WCHAR(s[c]) == L'\n') {
            c++;
            break;
        }

        while (t < w && QW_WCHAR(s[c + t]) != L' ' && QW_WCHAR(s[c + t]) != '\n')
            t++;

        while (t < w && QW_WCHAR(s[c + t]) == L' ')
            t++;

        c += t;
    }

    if (c > w)
        c -= t;

    return c;
}


void qw_char_set_attr(qw_char_t *s, off_t c, size_t z, qw_attr_t a)
{
    while (z--) {
        s[c] = QW_SET_ATTR(s[c], a);
        c++;
    }
}
