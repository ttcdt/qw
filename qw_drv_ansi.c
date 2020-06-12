/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#ifdef CONFOPT_ANSI

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <sys/time.h>

#include "qw_char.h"
#include "qw_core.h"

static int term_resized = 0;

static char ansi_attrs[QW_ATTR_COUNT][64];


static void ansi_raw_tty(int start)
/* sets/unsets stdin in raw mode */
{
    static struct termios so;

    if (start) {
        struct termios o;

        /* save previous fd state */
        tcgetattr(0, &so);

        /* set raw */
        tcgetattr(0, &o);
        cfmakeraw(&o);
        tcsetattr(0, TCSANOW, &o);
    }
    else
        /* restore previously saved tty state */
        tcsetattr(0, TCSANOW, &so);
}


static int ansi_something_waiting(int fd)
/* returns yes if there is something waiting on fd */
{
    fd_set ids;
    struct timeval tv;

    /* reset */
    FD_ZERO(&ids);

    /* add fd to set */
    FD_SET(fd, &ids);

    tv.tv_sec  = 0;
    tv.tv_usec = 10000;

    return select(1, &ids, NULL, NULL, &tv) > 0;
}


char *ansi_read_string(int fd)
/* reads an ansi string, waiting in the first char */
{
    static char *buf = NULL;
    static int z = 32;
    int n = 0;

    if (buf == NULL)
        buf = malloc(z);

    while (ansi_something_waiting(fd)) {
        char c;

        if (read(fd, &c, sizeof(c)) == -1)
            break;
        else {
            if (n == z) {
                z += 32;
                buf = realloc(buf, z + 1);
            }

            buf[n++] = c;
        }
    }

    buf[n] = '\0';

    return n ? buf : NULL;
}


static void ansi_get_tty_size(int *w, int *h)
/* asks the tty for its size */
{
    char *buffer;
    int r = 3;

    /* magic line: save cursor position, move to stupid position,
       ask for current position and restore cursor position */
    printf("\0337\033[r\033[999;999H\033[6n\0338");
    fflush(stdout);

    /* retry, as sometimes the answer is delayed from the signal */
    while (r) {
        buffer = ansi_read_string(0);

        if (buffer) {
            sscanf(buffer, "\033[%d;%dR", h, w);
            break;
        }

        usleep(100);
        r--;
    }

    if (r == 0) {
        *w = 80;
        *h = 25;
    }
}


static void ansi_resize(qw_core_t *c)
{
    int w, h;

    ansi_get_tty_size(&w, &h);

    qw_core_new_fb(c, w, h);

    term_resized = 0;
}


static void ansi_sigwinch(int s)
/* SIGWINCH signal handler */
{
    struct sigaction sa;

    term_resized = 1;

    /* (re)attach signal */
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = ansi_sigwinch;
    sigaction(SIGWINCH, &sa, NULL);
}


static void ansi_gotoxy(int x, int y)
/* positions the cursor */
{
    printf("\033[%d;%dH", y + 1, x + 1);
}


static void ansi_clrscr(void)
/* clears the screen */
{
    printf("\033[2J");
}


static void ansi_print(qw_char_t *s, size_t z)
/* prints a string */
{
    char tmp[1024];

    s[z] = L'\0';
    wcstombs(tmp, s, sizeof(tmp));
    printf("%s", tmp);
}


struct _rgb2cur {
    int color;
    int r;
    int g;
    int b;
    int bold;
};

struct _rgb2cur rgb2cur[] = {
    { 0,  0x00, 0x00, 0x00, 0 },
    { 4,  0x00, 0x00, 0x80, 0 },
    { 4,  0x00, 0x00, 0xff, 1 },
    { 1,  0x80, 0x00, 0x00, 0 },
    { 1,  0xff, 0x00, 0x00, 1 },
    { 2,  0x00, 0x80, 0x00, 0 },
    { 2,  0x00, 0xff, 0x00, 1 },
    { 6,  0x00, 0xff, 0xff, 0 },
    { 6,  0x77, 0xff, 0xff, 0 },
    { 3,  0xff, 0xff, 0x00, 1 },
    { 5,  0x80, 0x00, 0x80, 0 },
    { 5,  0xff, 0x00, 0xff, 1 },
    { 7,  0xff, 0xff, 0xff, 1 },
    { -1, 0x00, 0x00, 0x00, 0 }
};


static int find_color(int col, int *bold)
{
    if (col != -1) {
        int n, r, g, b;
        int best = 0;
        int lastcol = 0x7fffffff;

        r = (col & 0xff0000) >> 16;
        g = (col & 0x00ff00) >> 8;
        b = (col & 0x0000ff);

        for (n = 0; rgb2cur[n].color != -1; n++) {
            struct _rgb2cur *r2c = &rgb2cur[n];
            int curcol = abs(r - r2c->r) + abs(g - r2c->g) + abs(b - r2c->b);

            if (curcol < lastcol) {
                lastcol = curcol;
                best = n;
            }
        }

        col = rgb2cur[best].color;
        if (rgb2cur[best].bold)
            *bold = 1;
    }
    else
        col =  9;

    return col;
}


void ansi_cf_color(qw_core_t *c, qw_attr_t a, int irgb, int prgb, int rev)
{
    int ic, pc, b = 0, cf = 0;

    ic = find_color(irgb, &b);
    pc = find_color(prgb, &b);

    if (rev)
        cf |= 0x01;
    if (b)
        cf |= 0x02;

    sprintf(ansi_attrs[a], "\033[0;%s%s%s%d;%dm",
        cf & 0x1 ? "7;" : "",
        cf & 0x2 ? "1;" : "",
        cf & 0x4 ? "4;" : "",
        ic + 30,
        pc + 40
    );
}


void ansi_paint(qw_core_t *c)
{
    int y;
    qw_attr_t a;
    qw_char_t *w;

    w = calloc(c->fb->w + 1, sizeof(qw_char_t));

    for (y = 0; y < c->fb->h; y++) {
        size_t z;
        off_t x = 0;

        while ((z = qw_fb_get(c->fb, &x, y, &a, w))) {
            ansi_gotoxy(x, y);
            printf("%s", ansi_attrs[a]);
            ansi_print(w, z);
            x += z;
        }
    }

    ansi_gotoxy(c->fb->w - 1, c->fb->h - 1);

    fflush(stdout);

    free(w);
}


struct _str_to_key {
    char *ansi_str;
    qw_key_t key;
} str_to_key[] = {
//    { "\033[A\033[A\033[A", L"mouse-wheel-up" },
//    { "\033[B\033[B\033[B", L"mouse-wheel-down" },
    { "\033[A",             QW_KEY_UP },
    { "\033[B",             QW_KEY_DOWN },
    { "\033[C",             QW_KEY_RIGHT },
    { "\033[D",             QW_KEY_LEFT },
    { "\033[5~",            QW_KEY_PGUP },
    { "\033[6~",            QW_KEY_PGDN },
    { "\033[H",             QW_KEY_HOME },
    { "\033[F",             QW_KEY_END },
    { "\033OP",             QW_KEY_F1 },
    { "\033OQ",             QW_KEY_F2 },
    { "\033OR",             QW_KEY_F3 },
    { "\033OS",             QW_KEY_F4 },
    { "\033[15~",           QW_KEY_F5 },
    { "\033[17~",           QW_KEY_F6 },
    { "\033[18~",           QW_KEY_F7 },
    { "\033[19~",           QW_KEY_F8 },
    { "\033[20~",           QW_KEY_F9 },
    { "\033[21~",           QW_KEY_F10 },
/*    { "\033[1;2P",          L"shift-f1" },
    { "\033[1;2Q",          L"shift-f2" },
    { "\033[1;2R",          L"shift-f3" },
    { "\033[1;2S",          L"shift-f4" },
    { "\033[15;2~",         L"shift-f5" },
    { "\033[17;2~",         L"shift-f6" },
    { "\033[18;2~",         L"shift-f7" },
    { "\033[19;2~",         L"shift-f8" },
    { "\033[20;2~",         L"shift-f9" },
    { "\033[21;2~",         L"shift-f10" },

    { "\033[1;5P",          L"ctrl-f1" },
    { "\033[1;5Q",          L"ctrl-f2" },
    { "\033[1;5R",          L"ctrl-f3" },
    { "\033[1;5S",          L"ctrl-f4" },
    { "\033[15;5~",         L"ctrl-f5" },
    { "\033[17;5~",         L"ctrl-f6" },
    { "\033[18;5~",         L"ctrl-f7" },
    { "\033[19;5~",         L"ctrl-f8" },
    { "\033[20;5~",         L"ctrl-f9" },
    { "\033[21;5~",         L"ctrl-f10" },
    { "\033[1;2A",          L"_shift-cursor-up" },
    { "\033[1;2B",          L"_shift-cursor-down" },
    { "\033[1;2C",          L"_shift-cursor-right" },
    { "\033[1;2D",          L"_shift-cursor-left" },*/
    { "\033[1;5A",          QW_KEY_CTRL_UP },
    { "\033[1;5B",          QW_KEY_CTRL_DOWN },
    { "\033[1;5C",          QW_KEY_CTRL_RIGHT },
    { "\033[1;5D",          QW_KEY_CTRL_LEFT },
    { "\033[1;5H",          QW_KEY_CTRL_HOME },
    { "\033[1;5F",          QW_KEY_CTRL_END },
    { "\033[1;3A",          QW_KEY_ALT_UP },
    { "\033[1;3B",          QW_KEY_ALT_DOWN },
    { "\033[1;3C",          QW_KEY_ALT_RIGHT },
    { "\033[1;3D",          QW_KEY_ALT_LEFT },
    { "\033[1;3H",          QW_KEY_ALT_HOME },
    { "\033[1;3F",          QW_KEY_ALT_END },
    { "\033[3~",            QW_KEY_DEL },
/*    { "\033[2~",            L"insert" },*/
    { "\033[Z",             QW_KEY_SHIFT_TAB },
    { "\033\r",             QW_KEY_ALT_ENTER },
    { "\033[1~",            QW_KEY_HOME },
    { "\033[4~",            QW_KEY_END },
    { "\033[5;5~",          QW_KEY_CTRL_PGUP },
    { "\033[6;5~",          QW_KEY_CTRL_PGDN },
    { "\033[5;3~",          QW_KEY_ALT_PGUP },
    { "\033[6;3~",          QW_KEY_ALT_PGDN },
    { "\033-",              QW_KEY_ALT_MINUS },
    { "\033\033[A",         QW_KEY_ALT_UP },
    { "\033\033[B",         QW_KEY_ALT_DOWN },
    { "\033\033[C",         QW_KEY_ALT_RIGHT },
    { "\033\033[D",         QW_KEY_ALT_LEFT },
    { "\033\033[1~",        QW_KEY_ALT_HOME },
    { "\033\033[4~",        QW_KEY_ALT_END },
/*    { "\033\033[2~",        L"alt-insert" },
    { "\033\033[3~",        L"alt-delete" },*/
    { "\033\033[5~",        QW_KEY_ALT_PGUP },
    { "\033\033[6~",        QW_KEY_ALT_PGDN },
    { "\033OA",             QW_KEY_CTRL_UP },
    { "\033OB",             QW_KEY_CTRL_DOWN },
    { "\033OC",             QW_KEY_CTRL_RIGHT },
    { "\033OD",             QW_KEY_CTRL_LEFT },
    { NULL,                 QW_KEY_NONE }
};

#define ctrl(k) ((k) & 31)

static int ansi_key(qw_core_t *c)
{
    qw_key_t k = QW_KEY_NONE;
    qw_char_t wc = '\0';
    struct timeval t1, t2;
    char *str;

    gettimeofday(&t1, NULL);

    str = ansi_read_string(0);

    if (str) {
        /* only one char? it's an ASCII or ctrl character */
        if (str[1] == '\0') {
            if (str[0] == '\r')
                k = QW_KEY_ENTER;
            else
            if (str[0] == '\t')
                k = QW_KEY_TAB;
            else
            if (str[0] == '\033')
                k = QW_KEY_ESC;
            else
            if (str[0] == '\b' || str[0] == '\177')
                k = QW_KEY_BACKSPACE;
            else
            if (str[0] >= ctrl('a') && str[0] <= ctrl('z'))
                k = QW_KEY_CTRL_A + str[0] - ctrl('a');
            else
            if (str[0] == ctrl(' '))
                k = QW_KEY_CTRL_SPACE;
            else {
                k = QW_KEY_CHAR;
                wc = (qw_char_t) str[0];
            }
        }
        else {
            /* more than one char: possible ANSI sequence */
            int n;

            for (n = 0; str_to_key[n].ansi_str; n++) {
                if (strcmp(str_to_key[n].ansi_str, str) == 0) {
                    k = str_to_key[n].key;
                    break;
                }
            }

            /* not yet? convert the string to an wc */
            if (k == QW_KEY_NONE && str[0] != '\x1b') {
                mbtowc(&wc, str, strlen(str));
                k = QW_KEY_CHAR;
            }
        }

        if (k != QW_KEY_NONE)
            qw_core_new_key(c, k, wc);
    }

    if (term_resized)
        ansi_resize(c);

    gettimeofday(&t2, NULL);

    return ((t2.tv_sec  - t1.tv_sec) * 1000) +
           ((t2.tv_usec - t1.tv_usec) / 1000);
}


#define PAINT_PERIOD 100

void ansi_exec(qw_core_t *c)
{
    int msecs = 0;

    while (c->rf) {
        msecs += ansi_key(c);

        if (msecs >= PAINT_PERIOD) {
            msecs %= PAINT_PERIOD;

            if (qw_core_update(c))
                ansi_paint(c);
        }
    }

    ansi_raw_tty(0);

    ansi_clrscr();
}


static int ansi_startup(qw_core_t *c)
{
    signal(SIGPIPE, SIG_IGN);

    ansi_raw_tty(1);

    ansi_clrscr();

    ansi_sigwinch(0);

    ansi_resize(c);

    return 1;
}


int ansi_drv_detect(qw_core_t *c, int *argc, char ***argv)
{
    c->di       = L"ansi";
    c->exec     = ansi_exec;
    c->copy     = NULL;
    c->paste    = NULL;
    c->cf_color = ansi_cf_color;
    c->rf       = 1;

    return ansi_startup(c);
}

#endif /* CONFOPT_ANSI */
