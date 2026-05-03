#include "mnu.h"
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

/* Optimized Render Buffer for Zero Flicker */
typedef struct {
    char *data;
    size_t len;
    size_t cap;
} RenderBuf;

static void rb_init(RenderBuf *rb) {
    rb->cap = 16384; // Start with a larger buffer to avoid reallocs during draw
    rb->data = malloc(rb->cap);
    rb->len = 0;
    if (rb->data) rb->data[0] = '\0';
}

static void rb_append_str(RenderBuf *rb, const char *s) {
    if (!s) return;
    size_t slen = strlen(s);
    if (rb->len + slen >= rb->cap) {
        size_t new_cap = rb->cap * 2;
        while (rb->len + slen >= new_cap) new_cap *= 2;
        char *tmp = realloc(rb->data, new_cap);
        if (!tmp) return;
        rb->data = tmp;
        rb->cap = new_cap;
    }
    memcpy(rb->data + rb->len, s, slen);
    rb->len += slen;
    rb->data[rb->len] = '\0';
}

static void rb_printf(RenderBuf *rb, const char *fmt, ...) {
    if (!rb->data) return;
    va_list args;
    va_start(args, fmt);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed < 0) return;
    
    if (rb->len + (size_t)needed >= rb->cap) {
        size_t new_cap = rb->cap * 2;
        while (rb->len + (size_t)needed >= new_cap) new_cap *= 2;
        char *tmp = realloc(rb->data, new_cap);
        if (!tmp) return;
        rb->data = tmp;
        rb->cap = new_cap;
    }

    va_start(args, fmt);
    vsnprintf(rb->data + rb->len, (size_t)needed + 1, fmt, args);
    va_end(args);
    rb->len += (size_t)needed;
}

static void rb_flush(RenderBuf *rb) {
    if (rb->data && rb->len > 0) {
        write(STDOUT_FILENO, rb->data, rb->len);
        rb->len = 0;
    }
}

static void rb_free(RenderBuf *rb) {
    free(rb->data);
}

void renderer_draw(Document *doc, TerminalState *ts) {
    RenderBuf rb;
    rb_init(&rb);

    // 1. Hide cursor and move to home (1,1)
    rb_append_str(&rb, "\x1b[?25l\x1b[H");

    int viewport_height = ts->rows - 1;

    for (int i = 0; i < viewport_height; i++) {
        int line_idx = ts->scroll_y + i;
        
        // Ensure we are at the start of the current row (redundant but safe)
        // Actually, we rely on the previous line's \r\n
        
        if (line_idx < (int)doc->rendered.count) {
            rb_append_str(&rb, doc->rendered.lines[line_idx]);
        }
        
        // Clear to end of line and move to next row
        rb_append_str(&rb, "\x1b[K\r\n");
    }

    // 2. Footer: Position cursor at the last row
    rb_printf(&rb, "\x1b[%d;1H", ts->rows);
    
    // Use Inverted mode (7) - highly compatible and flicker-free
    rb_append_str(&rb, "\x1b[7m");
    
    const char *fmt = " MNU Pages: %s (q to quit) ";
    int foot_len = snprintf(NULL, 0, fmt, doc->page);
    
    if (foot_len > ts->cols) {
        // Truncate logic
        char *tmp = strdup(fmt); // placeholder for logic
        if (tmp) {
            // Manual truncation to be safe
            char foot_buf[1024];
            snprintf(foot_buf, sizeof(foot_buf), " MNU Pages: %s (q) ", doc->page);
            if ((int)strlen(foot_buf) > ts->cols) foot_buf[ts->cols] = '\0';
            rb_append_str(&rb, foot_buf);
            free(tmp);
        }
    } else {
        rb_printf(&rb, " MNU Pages: %s (q to quit) ", doc->page);
        for (int i = 0; i < ts->cols - foot_len; i++) rb_append_str(&rb, " ");
    }
    
    // 3. Reset formatting and flush
    rb_append_str(&rb, "\x1b[0m");

    rb_flush(&rb);
    rb_free(&rb);
}

void renderer_draw_footer(Document *doc, TerminalState *ts) {
    (void)doc; (void)ts;
}
