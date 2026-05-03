#include "mnu.h"
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <curl/curl.h>
#include <locale.h>

volatile sig_atomic_t resize_pending = 0;
volatile sig_atomic_t termination_pending = 0;

void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        termination_pending = 1;
    } else if (sig == SIGWINCH) {
        resize_pending = 1;
    }
}

static void exit_with_error(const char *message) {
    fprintf(stderr, "\x1b[31mError:\x1b[0m %s\n", message);
    exit(1);
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");

    if (argc != 2) {
        printf("Usage: mnu category:page\n");
        printf("Example: mnu cli:git\n");
        return 0;
    }

    curl_global_init(CURL_GLOBAL_ALL);

    char *arg = strdup(argv[1]);
    if (!arg) {
        curl_global_cleanup();
        return 1;
    }

    char *category = NULL;
    char *page = NULL;
    char *raw_data = NULL;
    int http_error = 0;

    if (strcmp(arg, "help") == 0 || strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
        category = "internal";
        page = "help";
        raw_data = strdup(help_get_content());
    } else {
        char *colon = strchr(arg, ':');
        if (!colon) {
            free(arg);
            exit_with_error("Invalid input format. Use category:page (e.g., cli:git)");
        }

        *colon = '\0';
        category = arg;
        page = colon + 1;

        // Match Node.js behavior: if there's a second colon, truncate it
        char *second_colon = strchr(page, ':');
        if (second_colon) *second_colon = '\0';

        if (strlen(category) == 0 || strlen(page) == 0) {
            free(arg);
            exit_with_error("Invalid input format. Use category:page (e.g., cli:git)");
        }

        raw_data = http_fetch(category, page, &http_error);
    }
    
    if (!raw_data) {
        char err_msg[256];
        if (http_error == 404) {
            snprintf(err_msg, sizeof(err_msg), "Page not found (404)");
        } else if (http_error == -1) {
            snprintf(err_msg, sizeof(err_msg), "Network error");
        } else {
            snprintf(err_msg, sizeof(err_msg), "Failed to fetch: %d", http_error);
        }
        free(arg);
        exit_with_error(err_msg);
    }

    Document *doc = parser_parse(raw_data, category, page);
    free(raw_data);
    if (!doc) {
        free(arg);
        exit_with_error("Could not parse page content");
    }

    TerminalState ts;
    terminal_get_size(&ts);
    layout_build(doc, ts.cols);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGWINCH, &sa, NULL);

    terminal_setup(&ts);

    int running = 1;
    int needs_redraw = 1;

    while (running && !termination_pending) {
        if (resize_pending) {
            terminal_get_size(&ts);
            layout_build(doc, ts.cols);
            resize_pending = 0;
            
            int max_scroll = (int)doc->rendered.count - (ts.rows - 1);
            if (max_scroll < 0) max_scroll = 0;
            if (ts.scroll_y > max_scroll) ts.scroll_y = max_scroll;
            
            needs_redraw = 1;
        }

        if (needs_redraw) {
            renderer_draw(doc, &ts);
            needs_redraw = 0;
        }
        
        int key = terminal_read_key();
        if (key == -2) continue; // Signal received, re-loop to handle flags
        if (key == -1) break;    // Real error or EOF

        int max_scroll = (int)doc->rendered.count - (ts.rows - 1);
        if (max_scroll < 0) max_scroll = 0;

        switch (key) {
            case 'q':
                running = 0;
                break;
            case 'j':
                if (ts.scroll_y < max_scroll) {
                    ts.scroll_y++;
                    needs_redraw = 1;
                }
                break;
            case 'k':
                if (ts.scroll_y > 0) {
                    ts.scroll_y--;
                    needs_redraw = 1;
                }
                break;
            case 'd': // Half page down
                if (ts.scroll_y < max_scroll) {
                    ts.scroll_y += (ts.rows / 2);
                    if (ts.scroll_y > max_scroll) ts.scroll_y = max_scroll;
                    needs_redraw = 1;
                }
                break;
            case 'u': // Half page up
                if (ts.scroll_y > 0) {
                    ts.scroll_y -= (ts.rows / 2);
                    if (ts.scroll_y < 0) ts.scroll_y = 0;
                    needs_redraw = 1;
                }
                break;
            case 'g':
            case KEY_HOME:
                if (ts.scroll_y != 0) {
                    ts.scroll_y = 0;
                    needs_redraw = 1;
                }
                break;
            case 'G':
            case KEY_END:
                if (ts.scroll_y != max_scroll) {
                    ts.scroll_y = max_scroll;
                    needs_redraw = 1;
                }
                break;
            case 'h':
                help_display(&ts);
                needs_redraw = 1;
                break;
        }
    }

    terminal_restore(&ts);
    parser_free(doc);
    free(arg);
    curl_global_cleanup();

    return 0;
}
