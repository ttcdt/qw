/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#ifdef CONFOPT_WIN32

#include <stdio.h>
#include <windows.h>

#include "qw_char.h"
#include "qw_core.h"
#include "qw_utf8.h"

static COLORREF inks[QW_ATTR_COUNT];
static COLORREF papers[QW_ATTR_COUNT];

static HFONT font = NULL;
static int font_width  = 0;
static int font_height = 0;


static void win32_resize(qw_core_t *c, HWND hwnd)
{
    if (font_width && font_height) {
        size_t tx, ty;
        RECT rect;

        GetClientRect(hwnd, &rect);

        tx = ((rect.right - rect.left) / font_width) + 1;
        ty = ((rect.bottom - rect.top) / font_height);

        qw_core_new_fb(c, tx, ty);
    }
}


void win32_exec(qw_core_t *c)
{
    MSG msg;

    while (c->rf && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


void win32_copy(qw_core_t *c)
{
    HGLOBAL hclp;
    WCHAR *p;

    hclp = GlobalAlloc(GHND, (c->cz + 1) * sizeof(WCHAR));
    p = (WCHAR *)GlobalLock(hclp);
    memcpy(p, c->cd, c->cz * sizeof(WCHAR));
    p[c->cz] = L'\0';
    GlobalUnlock(hclp);

    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, hclp);
    CloseClipboard();
}


void win32_paste(qw_core_t *c)
{
    HGLOBAL hclp;
    WCHAR *p;

    OpenClipboard(NULL);
    hclp = GetClipboardData(CF_UNICODETEXT);
    CloseClipboard();

    if (hclp && (p = GlobalLock(hclp)) != NULL) {
        c->cz = wcslen(p);
        c->cd = realloc(c->cd, (c->cz + 1) * sizeof(WCHAR));
        memcpy(c->cd, p, c->cz * sizeof(WCHAR));

        GlobalUnlock(hclp);
    }
}


void win32_cf_color(qw_core_t *c, qw_attr_t a, int irgb, int prgb, int rev)
{
    if (irgb == -1)
        irgb = 0x000000;
    if (prgb == -1)
        prgb = 0xffffff;

    if (rev) {
        int t = irgb;
        irgb = prgb;
        prgb = t;
    }

    inks[a]     = ((irgb & 0x000000ff) << 16) | ((irgb & 0x0000ff00)) | ((irgb & 0x00ff0000) >> 16);
    papers[a]   = ((prgb & 0x000000ff) << 16) | ((prgb & 0x0000ff00)) | ((prgb & 0x00ff0000) >> 16);
}


static void win32_build_font(qw_core_t *c, HWND hwnd)
{
    TEXTMETRIC tm;
    int font_size   = 10;
    char *font_face = "Lucida Console";
    HDC hdc;
    int i;

    hdc = GetDC(hwnd);

    i = -MulDiv(font_size, GetDeviceCaps(hdc, LOGPIXELSY), 72);

//    font = CreateFont(i, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, font_face);
    font = GetStockObject(SYSTEM_FIXED_FONT);

    SelectObject(hdc, font);
    GetTextMetrics(hdc, &tm);

    ReleaseDC(hwnd, hdc);

    font_height = tm.tmHeight;
    font_width  = tm.tmAveCharWidth;
}


static void win32_paint(qw_core_t *c, HWND hwnd)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;
    int y;
    qw_attr_t a;
    qw_char_t *w;

    w = calloc(c->fb->w + 1, sizeof(qw_char_t));

    hdc = BeginPaint(hwnd, &ps);
    GetClientRect(hwnd, &rect);
    SelectObject(hdc, font);

    for (y = 0; y < c->fb->h; y++) {
        size_t z;
        off_t x = 0;

        while ((z = qw_fb_get(c->fb, &x, y, &a, w))) {
            SetTextColor(hdc, inks[a]);
            SetBkColor(hdc, papers[a]);

            TextOutW(hdc, rect.left + x * font_width, rect.top + y * font_height, w, z);
            x += z;
        }
    }

    rect.top += c->fb->h * font_height;
    SetBkColor(hdc, papers[QW_ATTR_NORMAL]);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, "", 0, 0);

    EndPaint(hwnd, &ps);

    free(w);
}


static void win32_vkey(qw_core_t *c, int m)
{
    qw_key_t k = QW_KEY_NONE;
    qw_char_t wc = '\0';

    if (GetKeyState(VK_CONTROL) & 0x8000) {
        if (m == VK_UP)
            k = QW_KEY_CTRL_UP;
        else
        if (m == VK_DOWN)
            k = QW_KEY_CTRL_DOWN;
        else
        if (m == VK_LEFT)
            k = QW_KEY_CTRL_LEFT;
        else
        if (m == VK_RIGHT)
            k = QW_KEY_CTRL_RIGHT;
        else
        if (m == VK_PRIOR)
            k = QW_KEY_CTRL_PGUP;
        else
        if (m == VK_NEXT)
            k = QW_KEY_CTRL_PGDN;
        else
        if (m == VK_HOME)
            k = QW_KEY_CTRL_HOME;
        else
        if (m == VK_END)
            k = QW_KEY_CTRL_END;
        else
        if (m == VK_RETURN)
            k = QW_KEY_CTRL_ENTER;
        else
        if (m >= VK_F1 && m <= VK_F12)
            k = QW_KEY_CTRL_F1 + (m - VK_F1);
    }
    else
    if (GetKeyState(VK_LMENU) & 0x8000) {
        if (m == VK_UP)
            k = QW_KEY_ALT_UP;
        else
        if (m == VK_DOWN)
            k = QW_KEY_ALT_DOWN;
        else
        if (m == VK_LEFT)
            k = QW_KEY_ALT_LEFT;
        else
        if (m == VK_RIGHT)
            k = QW_KEY_ALT_RIGHT;
        else
        if (m == VK_PRIOR)
            k = QW_KEY_ALT_PGUP;
        else
        if (m == VK_NEXT)
            k = QW_KEY_ALT_PGDN;
        else
        if (m == VK_HOME)
            k = QW_KEY_ALT_HOME;
        else
        if (m == VK_END)
            k = QW_KEY_ALT_END;
        else
        if (m == VK_RETURN)
            k = QW_KEY_ALT_ENTER;
        else
        if (m >= VK_F1 && m <= VK_F12)
            k = QW_KEY_ALT_F1 + (m - VK_F1);
    }
    else {
        if (m == VK_UP)
            k = QW_KEY_UP;
        else
        if (m == VK_DOWN)
            k = QW_KEY_DOWN;
        else
        if (m == VK_LEFT)
            k = QW_KEY_LEFT;
        else
        if (m == VK_RIGHT)
            k = QW_KEY_RIGHT;
        else
        if (m == VK_PRIOR)
            k = QW_KEY_PGUP;
        else
        if (m == VK_NEXT)
            k = QW_KEY_PGDN;
        else
        if (m == VK_HOME)
            k = QW_KEY_HOME;
        else
        if (m == VK_END)
            k = QW_KEY_END;
        else
        if (m == VK_RETURN)
            k = QW_KEY_ENTER;
        else
        if (m == VK_BACK)
            k = QW_KEY_BACKSPACE;
        else
        if (m == VK_DELETE)
            k = QW_KEY_DEL;
        else
        if (m >= VK_F1 && m <= VK_F12)
            k = QW_KEY_F1 + (m - VK_F1);
    }

    if (k != QW_KEY_NONE)
        qw_core_new_key(c, k, wc);
}


#define ctrl(c) ((c) & 31)

static void win32_akey(qw_core_t *c, int m)
{
    qw_key_t k = QW_KEY_NONE;
    qw_char_t wc = '\0';

    if (m == ctrl('i')) {
        k = (GetKeyState(VK_SHIFT) & 0x8000) ? QW_KEY_SHIFT_TAB : QW_KEY_TAB;
    }
    else
    if (m >= ctrl('a') && m <= ctrl('z')) {
        k = QW_KEY_CTRL_A + (m - ctrl('a'));
    }
    else {
        k = QW_KEY_CHAR;
        wc = (qw_char_t)m;
    }

    if (k != QW_KEY_NONE)
        qw_core_new_key(c, k, wc);
}


static void win32_drop_files(qw_core_t *c, HDROP hDrop)
{
    int n;
    char tmp[16384];

    n = DragQueryFile(hDrop, 0xffffffff, NULL, sizeof(tmp) - 1);

    while (--n >= 0) {
        DragQueryFile(hDrop, n, tmp, sizeof(tmp) - 1);

        qw_core_open(c, tmp);
    }

    DragFinish(hDrop);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    qw_core_t *c;

    c = (qw_core_t *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE:
        win32_build_font(c, hwnd);
        DragAcceptFiles(hwnd, TRUE);
        return 0;

    case WM_PAINT:
        win32_paint(c, hwnd);
        return 0;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        win32_vkey(c, wparam);
        return 0;

    case WM_CHAR:
        win32_akey(c, wparam);
        return 0;

    case WM_SIZE:
        if (!IsIconic(hwnd)) {
            win32_resize(c, hwnd);
            InvalidateRect(hwnd, NULL, FALSE);
        }

        return 0;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_DROPFILES:
        win32_drop_files(c, (HDROP) wparam);
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_TIMER:
        if (qw_core_update(c))
            InvalidateRect(hwnd, NULL, FALSE);

        return 0;
    }

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}


static int win32_startup(qw_core_t *c)
{
    WNDCLASSW wc;
    HWND hwnd;
    HINSTANCE hinst;
    int ret = 1;

    hinst = GetModuleHandle(NULL);

    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hinst;
    wc.hIcon            = LoadIcon(hinst, "QW_ICON");
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = NULL;
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = L"qw";

    RegisterClassW(&wc);

    /* create the window */
    hwnd = CreateWindowW(L"qw", L"qw " VERSION,
                         WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                         CW_USEDEFAULT, CW_USEDEFAULT,
                         CW_USEDEFAULT, CW_USEDEFAULT,
                         NULL, NULL, hinst, NULL);

    if (hwnd) {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) c);

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        SetTimer(hwnd, 1, 100, NULL);
    }
    else
        ret = 0;

    return ret;
}


int win32_drv_detect(qw_core_t *c, int *argc, char ***argv)
{
    c->di       = L"win32";
    c->exec     = win32_exec;
    c->copy     = win32_copy;
    c->paste    = win32_paste;
    c->cf_color = win32_cf_color;
    c->rf       = 1;

    return win32_startup(c);
}

#endif /* CONFOPT_WIN32 */
