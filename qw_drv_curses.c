/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#ifdef CONFOPT_CURSES

#ifdef CONFOPT_NCURSESW_NCURSES
#include <ncursesw/ncurses.h>
#else
#include <curses.h>
#endif

#include <sys/time.h>
#include <string.h>

#include "qw_char.h"
#include "qw_core.h"

static int nc_attrs[QW_ATTR_COUNT];


/* prototypes NOT in ncurses.h... why? */
int waddnwstr(WINDOW *win, const wchar_t *wstr, int n);
int wget_wch(WINDOW *win, wint_t *wch);


static void curses_resize(qw_core_t *c)
{
    qw_core_new_fb(c, COLS, LINES);
}


struct _rgb2cur {
    int color;
    int r;
    int g;
    int b;
    int bold;
};

struct _rgb2cur rgb2cur[] = {
    { COLOR_BLACK,  0x00, 0x00, 0x00, 0 },
    { COLOR_BLUE,   0x00, 0x00, 0x80, 0 },
    { COLOR_BLUE,   0x00, 0x00, 0xff, 1 },
    { COLOR_RED,    0x80, 0x00, 0x00, 0 },
    { COLOR_RED,    0xff, 0x00, 0x00, 1 },
    { COLOR_GREEN,  0x00, 0x80, 0x00, 0 },
    { COLOR_GREEN,  0x00, 0xff, 0x00, 1 },
    { COLOR_CYAN,   0x00, 0xff, 0xff, 0 },
    { COLOR_CYAN,   0x77, 0xff, 0xff, 0 },
    { COLOR_YELLOW, 0xff, 0xff, 0x00, 1 },
    { COLOR_MAGENTA,0x80, 0x00, 0x80, 0 },
    { COLOR_MAGENTA,0xff, 0x00, 0xff, 1 },
    { COLOR_WHITE,  0xff, 0xff, 0xff, 1 },
    { -1,           0x00, 0x00, 0x00, 0 }
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

    return col;
}


void curses_cf_color(qw_core_t *c, qw_attr_t a, int irgb, int prgb, int rev)
{
    int ic, pc, b = 0;

    ic = find_color(irgb, &b);
    pc = find_color(prgb, &b);

    init_pair(a + 1, ic, pc);
    nc_attrs[a] = COLOR_PAIR(a + 1) | (rev ? A_REVERSE : 0) | (b ? A_BOLD : 0);
}


void curses_paint(qw_core_t *c)
{
    int y;
    qw_attr_t a;
    qw_char_t *w;

    w = calloc(c->fb->w + 1, sizeof(qw_char_t));

    for (y = 0; y < c->fb->h; y++) {
        size_t z;
        off_t x = 0;

        while ((z = qw_fb_get(c->fb, &x, y, &a, w))) {
            wmove(stdscr, y, x);
            wattrset(stdscr, nc_attrs[a]);
            waddnwstr(stdscr, w, z);
            x += z;
        }
    }

    wmove(stdscr, c->fb->h - 1, c->fb->w - 1);

    wrefresh(stdscr);

    free(w);
}


#define ctrl(k) ((k) & 31)

static int curses_key(qw_core_t *c)
{
    static int shift = 0;
    qw_key_t k = QW_KEY_NONE;
    qw_char_t wc = '\0';
    wchar_t s;
    struct timeval t1, t2;

    gettimeofday(&t1, NULL);

    if (wget_wch(stdscr, (wint_t *)&s) != -1) {
        if (!shift) {
            if (s == KEY_LEFT)
                 k = QW_KEY_LEFT;
            else
            if (s == KEY_RIGHT)
                k = QW_KEY_RIGHT;
            else
            if (s == KEY_UP)
                k = QW_KEY_UP;
            else
            if (s == KEY_DOWN)
                k = QW_KEY_DOWN;
            else
            if (s == KEY_PPAGE)
                k = QW_KEY_PGUP;
            else
            if (s == KEY_NPAGE)
                k = QW_KEY_PGDN;
            else
            if (s == KEY_HOME)
                k = QW_KEY_HOME;
            else
            if (s == KEY_END || s == KEY_LL)
                k = QW_KEY_END;
            else
            if (s == KEY_IC)
                k = QW_KEY_INSERT;
            else
            if (s == KEY_DC)
                k = QW_KEY_DEL;
            else
            if (s == 0x7f || s == KEY_BACKSPACE || s == L'\b')
                k = QW_KEY_BACKSPACE;
            else
            if (s == L'\r' || s == KEY_ENTER)
                k = QW_KEY_ENTER;
            else
            if (s == L'\t')
                k = QW_KEY_TAB;
            else
            if (s == KEY_BTAB)
                k = QW_KEY_SHIFT_TAB;
            else
            if (s == L'\e')
                shift = 1;
            else
            if (s >= KEY_F(1) && s <= KEY_F(10))
                k = QW_KEY_F1 + (s - KEY_F(1));
            else
            if (s == ctrl(' '))
                k = QW_KEY_CTRL_SPACE;
            else
            if (s >= ctrl('a') && s <= ctrl('z'))
                k = QW_KEY_CTRL_A + (s - ctrl('a'));
            else
            if (s == KEY_RESIZE)
                curses_resize(c);
            else {
                k = QW_KEY_CHAR;
                wc = (qw_char_t)s;
            }
        }
        else {
            if (s >= L'0' && s <= L'9')
                k = QW_KEY_ALT_F1 + (s - L'0');
            else
            if (s == KEY_LEFT)
                 k = QW_KEY_ALT_LEFT;
            else
            if (s == KEY_RIGHT)
                k = QW_KEY_ALT_RIGHT;
            else
            if (s == KEY_UP)
                k = QW_KEY_ALT_UP;
            else
            if (s == KEY_DOWN)
                k = QW_KEY_ALT_DOWN;
            else
            if (s == KEY_PPAGE)
                k = QW_KEY_ALT_PGUP;
            else
            if (s == KEY_NPAGE)
                k = QW_KEY_ALT_PGDN;
            else
            if (s == KEY_HOME)
                k = QW_KEY_ALT_HOME;
            else
            if (s == KEY_END || s == KEY_LL)
                k = QW_KEY_ALT_END;
            else
            if (s == L'\r' || s == KEY_ENTER)
                k = QW_KEY_ALT_ENTER;
            else
            if (s == L' ')
                k = QW_KEY_ALT_SPACE;
            else
            if (s >= L'a' && s <= L'z')
                k = QW_KEY_ALT_A + (s - L'a');
            else
            if (s == L'\t')
                k = QW_KEY_SHIFT_TAB;

            shift = 0;
        }

        if (k != QW_KEY_NONE)
            qw_core_new_key(c, k, wc);
    }

    gettimeofday(&t2, NULL);

    return ((t2.tv_sec  - t1.tv_sec) * 1000) +
           ((t2.tv_usec - t1.tv_usec) / 1000);
}


#define PAINT_PERIOD 100

void curses_exec(qw_core_t *c)
{
    int msecs = 0;

    timeout(PAINT_PERIOD);

    while (c->rf) {
        msecs += curses_key(c);

        if (msecs >= PAINT_PERIOD) {
            msecs %= PAINT_PERIOD;

            if (qw_core_update(c))
                curses_paint(c);
        }
    }

    endwin();
}


static int curses_startup(qw_core_t *c)
{
    initscr();
    start_color();

    keypad(stdscr, TRUE);
    nonl();
    raw();
    noecho();

    use_default_colors();

    curses_resize(c);

    return 1;
}


int curses_drv_detect(qw_core_t *c, int *argc, char ***argv)
{
    c->di       = L"curses";
    c->exec     = curses_exec;
    c->copy     = NULL;
    c->paste    = NULL;
    c->cf_color = curses_cf_color;
    c->rf       = 1;

    return curses_startup(c);
}

#endif /* CONFOPT_CURSES */
