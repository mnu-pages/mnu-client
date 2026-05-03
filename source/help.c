#include "mnu.h"

static const char *help_mn = 
    ".TITLE \"mnu help\"\n"
    "\n"
    ".DIV \"NAVIGATION\"\n"
    "j, ArrowDown : Scroll down\n"
    "k, ArrowUp   : Scroll up\n"
    "g            : Go to top\n"
    "G            : Go to bottom\n"
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

void help_display(TerminalState *ts) {
    Document *help_doc = parser_parse(help_mn, "internal", "help");
    if (!help_doc) return;

    layout_build(help_doc, ts->cols);
    
    int old_scroll = ts->scroll_y;
    ts->scroll_y = 0;

    int running = 1;
    while (running) {
        renderer_draw(help_doc, ts);
        int key = terminal_read_key();
        switch (key) {
            case 'q': case 'h': case '\x1b':
                running = 0;
                break;
            case 'j':
                if (ts->scroll_y < (int)help_doc->rendered.count - (ts->rows - 1))
                    ts->scroll_y++;
                break;
            case 'k':
                if (ts->scroll_y > 0)
                    ts->scroll_y--;
                break;
            case 'g':
                ts->scroll_y = 0;
                break;
            case 'G':
                ts->scroll_y = (int)help_doc->rendered.count - (ts->rows - 1);
                if (ts->scroll_y < 0) ts->scroll_y = 0;
                break;
        }
    }

    ts->scroll_y = old_scroll;
    parser_free(help_doc);
}
