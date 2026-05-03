#include "mnu.h"

const char *help_get_content(void) {
    return 
        ".TITLE \"mnu help\"\n"
        "\n"
        ".DIV \"WHAT IS MNU?\"\n"
        "mnu is your friendly tool for reading simple documentation right in your terminal. It's built to be fast, clean, and very easy to read.\n"
        "\n"
        ".DIV \"WHERE TO LOOK?\"\n"
        "We organize pages into different categories to make them easy to find:\n"
        "- **cli**: All about command-line tools and utilities.\n"
        "- **linux**: Helpful things and commands specifically for Linux.\n"
        "- **mac**: Helpful things and commands specifically for macOS.\n"
        "- **windows**: Helpful things and commands specifically for Windows.\n"
        "\n"
        ".DIV \"HOW TO MOVE?\"\n"
        "Getting around is simple! Just use these keys on your keyboard:\n"
        "- **j** or **Down**: Move down one line at a time.\n"
        "- **k** or **Up**: Move up one line at a time.\n"
        "- **d**: Jump down half a page to go faster.\n"
        "- **u**: Jump up half a page to go faster.\n"
        "- **g** or **Home**: Jump all the way to the top.\n"
        "- **G** or **End**: Jump all the way to the bottom.\n"
        "- **q**: Quit the viewer and go back to your shell.\n"
        "- **h**: Close this help guide.\n"
        "\n"
        ".DIV \"HOW TO USE?\"\n"
        "To read a page, just type the category and the page name like this:\n"
        "**mnu category:page**\n"
        "\n"
        "For example, to see how to use git, type: **mnu cli:git**\n"
        "\n"
        ".DIV \"WANT TO HELP?\"\n"
        "Found a bug, need to update mnu, or want to add a page of your own? Check these links:\n"
        "Project/Updates: __https://github.com/mnu-pages/mnu-client__\n"
        "Contribute: __https://github.com/mnu-pages__\n"
        "\n"
        ".DIV \"NOTE\"\n"
        "Right now, MNU Pages has a very limited number of pages. We welcome everyone to help us grow by contributing your own pages to the collection!\n";
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
