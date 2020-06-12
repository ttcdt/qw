/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#include "config.h"

#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <locale.h>

#include "qw.h"

/* total number of tests and oks */
int tests = 0;
int oks = 0;

/* failed tests messages */
char *failed_msgs[5000];
int i_failed_msgs = 0;

int verbose = 0;
int _do_benchmarks = 0;

void _do_test(char *desc, int res, int src_line)
{
    tests++;

    if (res) {
        oks++;
//        printf("OK\n");
        if (verbose)
            printf("stress.c:%d: #%d %s: OK\n", src_line, tests, desc);
    }
    else {
        printf("stress.c:%d: #%d error: %s *** Failed ***\n", src_line, tests, desc);
    }
}


int test_summary(void)
{
    printf("\n*** Total tests passed: %d/%d\n", oks, tests);

    if (oks == tests)
        printf("*** ALL TESTS PASSED\n");
    else {
        printf("*** %d %s\n", tests - oks, "TESTS ---FAILED---");

/*        printf("\nFailed tests:\n\n");
        for (n = 0; n < i_failed_msgs; n++)
            printf("%s", failed_msgs[n]);*/
    }

    return oks == tests ? 0 : 1;
}


#define do_test(d, r) _do_test(d, r, __LINE__)


double diff_time(struct timeval *st, struct timeval *et)
{
    double t = 0.0;

    if (et == NULL)
        gettimeofday(st, NULL);
    else {
        gettimeofday(et, NULL);

        t = et->tv_sec - st->tv_sec + (et->tv_usec - st->tv_usec) / 1000000.0;
    }

    return t;
}


void print_fb(qw_fb_t *fb)
{
    int n;

    for (n = 0; n < fb->w * fb->h; n++) {
        if (fb->d[n] == L'\n')
            printf("/");
        else
            printf("%lc", fb->d[n]);

        if (n % fb->w == fb->w - 1)
            printf("|\n");

    }
    printf("\n");
}


#define STRLEN 4096

void test_basic(void)
{
    qw_char_t str[STRLEN];
    size_t z;
    qw_char_t *ptr;
    qw_blk_t *b;
    off_t i, q;
    qw_char_t c;
    qw_jrnl_t *j = NULL;
    FILE *f;
    qw_fb_t *fb, *sfb;
    qw_edit_t *e;

    b = qw_blk_new(NULL, 0, 0, NULL, NULL);
    do_test("qw_blk_new", b != NULL);

    b = qw_blk_insert(b, 0, L"12345", 5);
    b = qw_blk_insert(b, 5, L"ABCDE", 5);
    z = qw_blk_get(b, 0, str, STRLEN);
    do_test("sizeof get 1", z == 10);
    do_test("insert & get 1", wcsncmp(str, L"12345ABCDE", 10) == 0);
    do_test("blk integrity 1", b->p == NULL && b->n == NULL);

    b = qw_blk_insert(b, 0, L"abcde", 5);
    z = qw_blk_get(b, 0, str, STRLEN);
    do_test("sizeof get 2", z == 15);
    do_test("insert & get 2", wcsncmp(str, L"abcde12345ABCDE", 15) == 0);
    do_test("blk integrity 2", b->p == NULL && b->n != NULL);

    z = qw_blk_get(b->n, 0, str, STRLEN);
    do_test("sizeof get 3", z == 10);
    do_test("insert & get 3", wcsncmp(str, L"12345ABCDE", 10) == 0);

    b = qw_blk_insert(b, 2, L"---", 3);
    z = qw_blk_get(b, 0, str, STRLEN);
    do_test("sizeof get 3", z == 18);
    do_test("insert & get 3", wcsncmp(str, L"ab---cde12345ABCDE", z) == 0);

    b = qw_blk_insert(b->n, 0, L"!!!", 3);
    b = qw_blk_first(b);
    z = qw_blk_get(b, 0, str, STRLEN);
    do_test("sizeof get 4", z == 21);
    do_test("insert & get 4", wcsncmp(str, L"ab---!!!cde12345ABCDE", z) == 0);

    b = qw_blk_move(b, 0, 1, &i);
    qw_blk_get(b, i, &c, 1);
    do_test("fwd 1", c == L'b');
    b = qw_blk_move(b, i, 1, &i);
    qw_blk_get(b, i, &c, 1);
    do_test("fwd 2", c == L'-');
    b = qw_blk_move(b, i, 13, &i);
    qw_blk_get(b, i, &c, 1);
    do_test("fwd 3", c == L'5');
    do_test("fwd (now in a different block)", b != qw_blk_first(b));
    b = qw_blk_move(b, i, 3, &i);
    qw_blk_get(b, i, &c, 1);
    do_test("fwd 4", c == L'C');
    do_test("fwd 5 beyond end returns NULL", qw_blk_move(b, i, 100, &i) == NULL);
    b = qw_blk_move(b, i, 1, &i);
    qw_blk_get(b, i, &c, 1);
    do_test("fwd 6", c == L'D');
    b = qw_blk_move(b, i, 1, &i);
    qw_blk_get(b, i, &c, 1);
    do_test("fwd 7", c == L'E');
    b = qw_blk_move(b, i, 1, &i);
    do_test("fwd 8.1 move to EOF succeeds", b != NULL);
    z = qw_blk_get(b, i, &c, 1);
    do_test("fwd 8.2 but a get returns zero", z == 0);
    do_test("fwd 9 moving further fails as expected", qw_blk_move(b, i, 1, &i) == NULL);

    b = qw_blk_move(b, i, -1, &i);
    z = qw_blk_get(b, i, &c, 1);
    do_test("bwd 1", c == L'E');
    b = qw_blk_move(b, i, -2, &i);
    z = qw_blk_get(b, i, &c, 1);
    do_test("bwd 2", c == L'C');
    b = qw_blk_move(b, i, -10, &i);
    z = qw_blk_get(b, i, &c, 1);
    do_test("bwd 3", c == L'c');
    do_test("bwd 4 beyond start returns NULL", qw_blk_move(b, i, -15, &i) == NULL);

    b = qw_blk_to_rel(b, 15, &i);
    do_test("absolute offset 1", qw_blk_to_abs(b, i) == 15);
    b = qw_blk_move(b, i, -5, &i);
    do_test("absolute offset 2", qw_blk_to_abs(b, i) == 10);
    b = qw_blk_move(b, i, -5, &i);
    do_test("absolute offset 3", qw_blk_to_abs(b, i) == 5);
    b = qw_blk_move(b, i, -5, &i);
    do_test("absolute offset 4", qw_blk_to_abs(b, i) == 0);

    b = qw_blk_first(b);
    do_test("qw_blk_here() 1", qw_blk_here(b, 0, &i, L"ab", 2) != NULL);
    do_test("qw_blk_here() 2", qw_blk_here(b, 1, &i, L"ab", 2) == NULL);
    do_test("qw_blk_here() 3", qw_blk_here(b, 0, &i, L"ab---!!!cde", 11) != NULL);
    do_test("qw_blk_here() 4", qw_blk_here(b, 2, &i, L"---!!!cde12", 11) != NULL);

    b = qw_blk_first(b);
    do_test("search fwd 1", qw_blk_search(b, 0, &i, L"ABC", 3, 1) != NULL);
    do_test("search fwd 2", qw_blk_search(b, 0, &i, L"XYZ", 3, 1) == NULL);
    b = qw_blk_move(b, 0, 15, &i);
    do_test("search bwd 1", qw_blk_search(b, i, &i, L"345", 3, -1) != NULL);
    do_test("search bwd 2", qw_blk_search(b, i, &i, L"ab", 2, -1) != NULL);
    b = qw_blk_first(b);
    b = qw_blk_search(b, 0, &i, L"ABC", 3, 1);
    z = qw_blk_get(b, i, &c, 1);
    do_test("search is set after the last found char 1", c == L'D');
    b = qw_blk_search(b, i, &i, L"A", 1, -1);
    z = qw_blk_get(b, i, &c, 1);
    do_test("search is set after the last found char 2", c == L'B');

    b = qw_blk_first(b);

    ptr = L"this is a test";
    b = qw_blk_insert(b, 0, ptr, wcslen(ptr));

    ptr = L"abcdefghijklmnopqrstuvwxyz";

    i = qw_blk_i_and_not_e(b, 0, 1, ptr, NULL);
    z = qw_blk_get(b, 0, str, i + 1);
    str[i + 1] = L'\0';
    do_test("i_and_not_e 1", wcscmp(str, L"this") == 0);
    i += 2;
    q = qw_blk_i_and_not_e(b, i, 1, ptr, NULL);
    z = qw_blk_get(b, i, str, q - i + 1);
    str[q - i + 1] = L'\0';
    do_test("i_and_not_e 2", wcscmp(str, L"is") == 0);

    q = qw_blk_i_and_not_e(b, q, -1, ptr, NULL);
    do_test("i_and_not_e 3", i == q);

    i -= 2;
    q = qw_blk_i_and_not_e(b, i, -1, ptr, NULL);
    do_test("i_and_not_e 4", q == 0);

    i = qw_blk_i_and_not_e(b, q, 1, NULL, L" ") + 1;
    z = qw_blk_get(b, 0, str, i);
    str[i] = L'\0';
    do_test("i_and_not_e 5", wcscmp(str, L"this") == 0);
    q = qw_blk_i_and_not_e(b, i, 1, L" ", NULL) + 1;
    i = qw_blk_i_and_not_e(b, q, 1, NULL, L" ") + 1;
    z = qw_blk_get(b, q, str, i - q);
    str[i - q] = 0;
    do_test("i_and_not_e 6", wcscmp(str, L"is") == 0);

    qw_blk_destroy(qw_blk_first(b));

    /* journal */
    b = qw_blk_new(NULL, 0, 0, NULL, NULL);

    ptr = L"new string";
    j = qw_jrnl_new(1, b, 0, ptr, wcslen(ptr), j);
    b = qw_jrnl_apply(b, j, 1);
    z = qw_blk_get(qw_blk_first(b), 0, str, STRLEN);
    do_test("jrnl 1 (insert 'new string')", wcsncmp(str, ptr, z) == 0);
    ptr = L"\nmore";
    j = qw_jrnl_new(1, b, b->l, ptr, wcslen(ptr), j);
    b = qw_jrnl_apply(b, j, 1);
    z = qw_blk_get(qw_blk_first(b), 0, str, STRLEN);
    do_test("jrnl 2 (insert 'more')", wcsncmp(str, L"new string\nmore", z) == 0);
    j = qw_jrnl_new(0, b, 0, NULL, 3, j);
    b = qw_jrnl_apply(b, j, 1);
    z = qw_blk_get(qw_blk_first(b), 0, str, 7);
    do_test("jrnl 3 (delete 'new ')", wcsncmp(str, L" string", z) == 0);
    ptr = L"incredible";
    j = qw_jrnl_new(1, b, 0, ptr, wcslen(ptr), j);
    b = qw_jrnl_apply(b, j, 1);
    z = qw_blk_get(qw_blk_first(b), 0, str, STRLEN);
    do_test("jrnl 4 (insert again)", wcsncmp(str, L"incredible string\nmore", z) == 0);

    /* undo */
    b = qw_jrnl_apply(b, j, 0);
    j = j->p;
    z = qw_blk_get(b, 0, str, 7);
    do_test("jrnl 5 (undo 4)", wcsncmp(str, L" string", z) == 0);

    b = qw_jrnl_apply(b, j, 0);
    j = j->p;
    z = qw_blk_get(b, 0, str, STRLEN);
    do_test("jrnl 6 (undo 3)", wcsncmp(str, L"new string\nmore", z) == 0);

    /* redo */
    j = j->n;
    b = qw_jrnl_apply(b, j, 1);
    z = qw_blk_get(b, 0, str, 7);
    do_test("jrnl 7 (redo)", wcsncmp(str, L" string", z) == 0);

    j = j->n;
    b = qw_jrnl_apply(b, j, 1);
    z = qw_blk_get(b, 0, str, STRLEN);
    do_test("jrnl 8 (redo)", wcsncmp(str, L"incredible string\nmore", z) == 0);

    /* undo again */
    b = qw_jrnl_apply(b, j, 0);
    j = j->p;
    z = qw_blk_get(qw_blk_first(b), 0, str, 7);
    do_test("jrnl 9 (undo 4)", wcsncmp(str, L" string", z) == 0);

    b = qw_jrnl_apply(b, j, 0);
    j = j->p;
    z = qw_blk_get(qw_blk_first(b), 0, str, STRLEN);
    do_test("jrnl 10 (undo 3)", wcsncmp(str, L"new string\nmore", z) == 0);

    /* new journal entry (all undone is lost) */
    b = qw_blk_move(qw_blk_first(b), 0, 3, &i);
    j = qw_jrnl_new(0, b, 3, NULL, 7, j);
    b = qw_jrnl_apply(b, j, 1);
    z = qw_blk_get(qw_blk_first(b), 0, str, STRLEN);
    do_test("jrnl 11 (new branch, delete 'string')", wcsncmp(str, L"new\nmore", z) == 0);

    b = qw_jrnl_apply(b, j, 0);
    j = j->p;
    z = qw_blk_get(qw_blk_first(b), 0, str, STRLEN);
    do_test("jrnl 12 (undo)", wcsncmp(str, L"new string\nmore", z) == 0);

    b = qw_jrnl_apply(b, j, 0);
    j = j->p;
    z = qw_blk_get(qw_blk_first(b), 0, str, STRLEN);
    do_test("jrnl 13 (back to first content)", wcsncmp(str, L"new string", z) == 0);

    b = qw_jrnl_apply(b, j, 0);
    z = qw_blk_get(qw_blk_first(b), 0, str, STRLEN);
    do_test("jrnl 14 (back to empty)", z == 0);

    b = qw_blk_destroy(b);

    int cr = 0;

    if ((f = fopen("stress.txt", "rb")) != NULL) {
        i = 0;
        while ((z = qw_utf8_read(f, str, STRLEN, &cr))) {
            b = qw_blk_insert(b, i, str, z);
            b = qw_blk_move(b, i, z, &i);
        }

        fclose(f);
    }

    b = qw_blk_insert(qw_blk_first(b), 5, L"¡ÑÁ!", 4);

    if ((f = fopen("stress.out", "wb")) != NULL) {
        i = 0;
        b = qw_blk_first(b);
        while ((z = qw_blk_get(b, i, str, STRLEN))) {
            qw_utf8_write(f, str, z, cr);
            b = qw_blk_move(b, i, z, &i);
        }

        fclose(f);
    }

    b = qw_blk_destroy(b);

    /* fb */
    ptr = L"This is a rather long string to test the qw framebuffer engine,\nthat shall wrap by word. Thisisanonexistentverylongword that is not wrappable.";

    fb = qw_fb_new(23, 9);
    qw_fb_put(fb, ptr, wcslen(ptr));

//    print_fb(fb);
    sfb = qw_fb_new(60, 20);
    sfb->f = L'#';
    qw_fb_put(sfb, NULL, 0);
    qw_fb_mix(sfb, fb, 5, 5);

    if (verbose)
        print_fb(sfb);

    /* edit */
    e = qw_edit_new("stress.txt", NULL);

    if ((f = fopen(e->fn, "rb")) != NULL) {
        i = 0;
        while ((z = qw_utf8_read(f, str, STRLEN, &cr))) {
            e->b = qw_blk_insert(e->b, i, str, z);
            e->b = qw_blk_move(e->b, i, z, &i);
        }

        fclose(f);
    }

    off_t av = 0;
    qw_edit_paint(e, &av, sfb);
    qw_fb_mix(sfb, fb, 5, 5);

    if (verbose)
        print_fb(sfb);

    /* qw_blk_next_row */

    b = qw_blk_destroy(b);
    b = qw_blk_insert(b, 0, L"ABCDE 12345 ", 12);
    i = qw_blk_row_next(b, 0, 10);
    do_test("qw_blk_next_row 1", i == 6);
    i = qw_blk_row_next(b, i, 10);
    do_test("qw_blk_next_row 2 (set to EOF)", i == 6);
    b = qw_blk_first(b);
    b = qw_blk_insert(b, 0, L"ab ", 3);
    i = qw_blk_row_next(b, 0, 10);
    b = qw_blk_move(b, 0, i, &i);
    z = qw_blk_get(b, i, str, 5);
    do_test("qw_blk_next_row 3", wcsncmp(str, L"12345", z) == 0);
    b = qw_blk_first(b);
    i = qw_blk_row_next(b, 0, 5);
    b = qw_blk_move(b, 0, i, &i);
    z = qw_blk_get(b, i, str, 5);
    do_test("qw_blk_next_row 4", wcsncmp(str, L"ABCDE", z) == 0);
    b = qw_blk_first(b);
    b = qw_blk_insert(b, 0, L"C\n", 2);
    i = qw_blk_row_next(b, 0, 10);
    b = qw_blk_move(b, 0, i, &i);
    z = qw_blk_get(b, i, str, 5);
    do_test("qw_blk_next_row 5", wcsncmp(str, L"ab AB", z) == 0);
    b = qw_blk_first(b);
    b = qw_blk_insert(b, 0, L"A \n", 3);
    i = qw_blk_row_next(b, 0, 10);
    b = qw_blk_move(b, 0, i, &i);
    z = qw_blk_get(b, i, str, 2);
    do_test("qw_blk_next_row 6", wcsncmp(str, L"C\n", z) == 0);

    b = qw_blk_destroy(b);
    b = qw_blk_insert(b, 0, L"ABCDE\n12345\n", 12);
    b = qw_blk_move_bol(b, 2, &i);
    z = qw_blk_get(b, i, &c, 1);
    do_test("qw_blk_move_bol 1", i == 0 && c == L'A');
    b = qw_blk_move_bol(b, 5, &i);
    z = qw_blk_get(b, i, &c, 1);
    do_test("qw_blk_move_bol 2", i == 0 && c == L'A');
    b = qw_blk_move_bol(b, 6, &i);
    z = qw_blk_get(b, i, &c, 1);
    do_test("qw_blk_move_bol 3", i == 6 && c == L'1');
    b = qw_blk_move_bol(b, 11, &i);
    z = qw_blk_get(b, i, &c, 1);
    do_test("qw_blk_move_bol 4", i == 6 && c == L'1');
    b = qw_blk_insert(b, 0, L"\n", 1);
    b = qw_blk_move_bol(b, 0, &i);
    z = qw_blk_get(b, i, &c, 1);
    do_test("qw_blk_move_bol 5", i == 0 && c == L'\n');
}


int main(int argc, char *argv[])
{
    int n;

    setlocale(LC_ALL, "");

    printf("qw %s stress tests\n", VERSION);

    for (n = 1; n < argc; n++) {
        if (strcmp(argv[n], "-b") == 0)
            _do_benchmarks = 1;
        if (strcmp(argv[n], "-v") == 0)
            verbose = 1;
    }

    test_basic();

    return test_summary();
}
