#include "mnu.h"

const char *help_get_content(void) {
    return 
        ".TITLE \"mnu help\"\n"
        "\n"
        ".DIV \"NAVIGATION\"\n"
        "j, ArrowDown : Scroll down\n"
        "k, ArrowUp   : Scroll up\n"
        "d            : Half page down\n"
        "u            : Half page up\n"
        "g, Home      : Go to top\n"
        "G, End       : Go to bottom\n"
        "q            : Quit viewer\n"
        "h            : Close help\n"
        "\n"
        ".DIV \"USAGE\"\n"
        "mnu category:page\n"
        "Example: **mnu cli:git**\n"
        "\n"
        ".DIV \"ABOUT\"\n"
        "mnu is a simple terminal client for reading .mn documentation pages.\n"
        "\n"
        "To contribute, visit:\n"
        "__https://github.com/mnu-pages__\n";
}

void help_display(TerminalState *ts) {
    Document *help_doc = parser_parse(help_get_content(), "internal", "help");
    if (!help_doc) return;

    layout_build(help_doc, ts->cols);
    
    int old_scroll = ts->scroll_y;
    ts->scroll_y = 0;

    int running = 1;
    while (running) {
        renderer_draw(help_doc, ts);
        int key = terminal_read_key();
        
        int max_scroll = (int)help_doc->rendered.count - (ts->rows - 1);
        if (max_scroll < 0) max_scroll = 0;

        switch (key) {
            case 'q': case 'h': case '\x1b':
                running = 0;
                break;
            case 'j':
                if (ts->scroll_y < max_scroll)
                    ts->scroll_y++;
                break;
            case 'k':
                if (ts->scroll_y > 0)
                    ts->scroll_y--;
                break;
            case 'd':
                if (ts->scroll_y < max_scroll) {
                    ts->scroll_y += (ts->rows / 2);
                    if (ts->scroll_y > max_scroll) ts->scroll_y = max_scroll;
                }
                break;
            case 'u':
                if (ts->scroll_y > 0) {
                    ts->scroll_y -= (ts->rows / 2);
                    if (ts->scroll_y < 0) ts->scroll_y = 0;
                }
                break;
            case 'g':
            case KEY_HOME:
                ts->scroll_y = 0;
                break;
            case 'G':
            case KEY_END:
                ts->scroll_y = max_scroll;
                break;
        }
    }

    ts->scroll_y = old_scroll;
    parser_free(help_doc);
}
