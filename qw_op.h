/*

    qw - A minimalistic text editor

    ttcdt <dev@triptico.com>

    This software is released into the public domain.
    NO WARRANTY. See file LICENSE for details.

*/

#ifndef QW_OP_H
#define QW_OP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    QW_OP_NOP,
    QW_OP_LEFT,
    QW_OP_RIGHT,
    QW_OP_UP,
    QW_OP_DOWN,
    QW_OP_PGUP,
    QW_OP_PGDN,
    QW_OP_BOR,
    QW_OP_EOR,
    QW_OP_BOL,
    QW_OP_EOL,
    QW_OP_BOF,
    QW_OP_EOF,
    QW_OP_CHAR,
    QW_OP_NEWLINE,
    QW_OP_TAB,
    QW_OP_HARD_TAB,
    QW_OP_DEL,
    QW_OP_BACKSPACE,
    QW_OP_UNDO,
    QW_OP_REDO,
    QW_OP_DEL_ROW,
    QW_OP_MARK,
    QW_OP_UNMARK,
    QW_OP_COPY,
    QW_OP_CUT,
    QW_OP_PASTE,
    QW_OP_NEXT,
    QW_OP_CLOSE,
    QW_OP_DESTROY,
    QW_OP_QUIT,
    QW_OP_SAVE,
    QW_OP_SHOW_CODES,
    QW_OP_SEARCH,
    QW_OP_NEW_SEARCH,
    QW_OP_M_DASH,
    QW_OP_COUNT
} qw_op_t;

#ifdef __cplusplus
}
#endif

#endif /* QW_OP_H */
