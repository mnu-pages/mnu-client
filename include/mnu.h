#ifndef MNU_H
#define MNU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

/* --- CONSTANTS --- */

#define MNU_VERSION "0.1.0"
#define MNU_BASE_URL "https://raw.githubusercontent.com/mnu-pages/pages/main"

/* --- KEY DEFINITIONS --- */

#define KEY_HOME 1001
#define KEY_END  1002

/* --- STRUCTURES --- */

typedef enum {
    LINE_TEXT,
    LINE_TITLE,
    LINE_DIV,
    LINE_EMPTY,
    LINE_SPACE
} LineType;

typedef struct {
    LineType type;
    char *content;
} Line;

typedef struct {
    char **lines;
    size_t count;
    size_t capacity;
} RenderedPage;

typedef struct {
    Line *elements;
    size_t element_count;
    size_t element_capacity;
    
    RenderedPage rendered;
    
    char *category;
    char *page;
} Document;

typedef struct {
    int rows;
    int cols;
    int scroll_y;
    struct termios orig_termios;
} TerminalState;

/* --- MODULES --- */

/* http.c */
char *http_fetch(const char *category, const char *page, int *error_code);

/* parser.c */
Document *parser_parse(const char *raw_data, const char *category, const char *page);
void parser_free(Document *doc);

/* layout.c */
void layout_build(Document *doc, int width);
void layout_free(Document *doc);

/* renderer.c */
void renderer_draw(Document *doc, TerminalState *ts);
void renderer_draw_footer(Document *doc, TerminalState *ts);

/* terminal.c */
void terminal_setup(TerminalState *ts);
void terminal_restore(TerminalState *ts);
void terminal_get_size(TerminalState *ts);
int terminal_read_key(void);
void terminal_clear(void);
void terminal_move_cursor(int row, int col);
void terminal_enter_alt_buffer(void);
void terminal_exit_alt_buffer(void);
void terminal_hide_cursor(void);
void terminal_show_cursor(void);

/* help.c */
const char *help_get_content(void);
void help_display(TerminalState *ts);

#endif
