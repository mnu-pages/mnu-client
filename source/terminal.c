#include "mnu.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

void terminal_setup(TerminalState *ts) {
    if (tcgetattr(STDIN_FILENO, &ts->orig_termios) == -1) return;

    struct termios raw = ts->orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) return;

    terminal_enter_alt_buffer();
    terminal_hide_cursor();
    terminal_get_size(ts);
    ts->scroll_y = 0;
}

void terminal_restore(TerminalState *ts) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &ts->orig_termios);
    terminal_exit_alt_buffer();
    terminal_show_cursor();
}

void terminal_get_size(TerminalState *ts) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        ts->rows = 24;
        ts->cols = 80;
    } else {
        ts->rows = ws.ws_row;
        ts->cols = ws.ws_col;
    }
}

int terminal_read_key(void) {
    char c;
    ssize_t nread;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1) {
            if (errno == EAGAIN || errno == EINTR) return -2; // Interrupted, not an error
            return -1;
        }
        if (nread == 0) return -1;
    }

    if (c == '\x1b') {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return 'k'; // Arrow Up
                case 'B': return 'j'; // Arrow Down
            }
        }
        return '\x1b';
    }

    return (int)c;
}

void terminal_clear(void) {
    const char *s = "\x1b[2J\x1b[H";
    write(STDOUT_FILENO, s, strlen(s));
}

void terminal_move_cursor(int row, int col) {
    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", row, col);
    write(STDOUT_FILENO, buf, strlen(buf));
}

void terminal_enter_alt_buffer(void) {
    const char *s = "\x1b[?1049h";
    write(STDOUT_FILENO, s, strlen(s));
}

void terminal_exit_alt_buffer(void) {
    const char *s = "\x1b[?1049l";
    write(STDOUT_FILENO, s, strlen(s));
}

void terminal_hide_cursor(void) {
    const char *s = "\x1b[?25l";
    write(STDOUT_FILENO, s, strlen(s));
}

void terminal_show_cursor(void) {
    const char *s = "\x1b[?25h";
    write(STDOUT_FILENO, s, strlen(s));
}
