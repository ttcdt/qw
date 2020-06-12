/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "qw_utf8.h"


size_t qw_utf8_read(FILE *f, qw_char_t *p, size_t z, int *cr)
{
    int c;
    qw_char_t wc;
    size_t n = 0;

    while (n < z) {
        wc = 0;

        if ((c = fgetc(f)) == EOF)
            break;

        if (c == '\r') {
            *cr = 1;
            continue;
        }

        if ((c & 0x80) == 0)
            wc = c;
        else
        if ((c & 0xe0) == 0xe0) {
            wc = (c & 0x1f) << 12;

            if ((c = fgetc(f)) == EOF)
                break;

            wc |= (c & 0x3f) << 6;

            if ((c = fgetc(f)) == EOF)
                break;

            wc |= (c & 0x3f);
        }
        else {
            wc = (c & 0x3f) << 6;

            if ((c = fgetc(f)) == EOF)
                break;

            wc |= (c & 0x3f);
        }

        p[n++] = wc;
    }

    return n;
}


void qw_utf8_write(FILE *f, const qw_char_t *p, size_t z, int cr)
{
    size_t n = 0;

    while (n < z) {
        int i, m;
        char mb[8];

        i = qw_utf8_wc_to_mb(QW_WCHAR(p[n++]), mb, cr);

        for (m = 0; m < i; m++)
            fputc((int) mb[m], f);
    }
}


int qw_utf8_wc_to_mb(const qw_char_t wc, char *mb, int cr)
{
    int r = 0;

    if (wc == L'\n') {
        if (cr) {
            mb[0] = '\r';
            mb[1] = '\n';
            r = 2;
        }
        else {
            mb[0] = '\n';
            r = 1;
        }
    }
    else
    if (wc < 0x80) {
        mb[0] = (char) (wc & 0x7f);
        r = 1;
    }
    else
    if (wc < 0x800) {
        mb[0] = (char) (0xc0 | (wc >> 6));
        mb[1] = (char) (0x80 | (wc & 0x3f));
        r = 2;
    }
    else {
        mb[0] = (char) (0xe0 | (wc >> 12));
        mb[1] = (char) (0x80 | ((wc >> 6) & 0x3f));
        mb[2] = (char) (0x80 | (wc & 0x3f));
        r = 3;
    }

    mb[r] = '\0';

    return r;
}

