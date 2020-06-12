/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#include <locale.h>

#include "qw.h"

#include "qw_avail_drv.h"

extern wchar_t *qw_def_cf[];

int main(int argc, char *argv[])
{
    int n;
    qw_core_t *c;

    c = qw_core_new();

    setlocale(LC_ALL, "");

    if (!TRY_DRIVERS()) {
        printf("Error: no driver found -- exiting\n");
        return 1;
    }

    for (n = 0; qw_def_cf[n]; n++)
        qw_cf_cmd(c, qw_def_cf[n]);

    for (n = 1; n < argc; n++) {
        char *fn = argv[n];

        if (*fn != '-')
            qw_core_open(c, fn);
    }

    c->exec(c);

    return 0;
}
