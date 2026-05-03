#include "mnu.h"
#include <string.h>
#include <stdarg.h>

#include <wchar.h>
#include <locale.h>

/* Dynamic Buffer Utility */
typedef struct {
    char *data;
    size_t len;
    size_t cap;
} DynBuf;

static int db_init(DynBuf *db) {
    db->cap = 128;
    db->len = 0;
    db->data = malloc(db->cap);
    if (!db->data) {
        db->cap = 0;
        return 0;
    }
    db->data[0] = '\0';
    return 1;
}

static void db_clear(DynBuf *db) {
    db->len = 0;
    if (db->data) db->data[0] = '\0';
}

static int db_ensure(DynBuf *db, size_t add) {
    if (!db->data && db->cap == 0) {
        db->cap = add > 128 ? add + 1 : 128;
        db->data = malloc(db->cap);
        if (!db->data) return 0;
        db->data[0] = '\0';
    }
    if (db->len + add >= db->cap) {
        size_t new_cap = db->cap > 0 ? db->cap * 2 : 128;
        while (db->len + add >= new_cap) new_cap *= 2;
        char *tmp = realloc(db->data, new_cap);
        if (!tmp) return 0;
        db->data = tmp;
        db->cap = new_cap;
    }
    return 1;
}

static int db_append_n(DynBuf *db, const char *s, size_t n) {
    if (!s || n == 0) return 1;
    if (!db_ensure(db, n)) return 0;
    memcpy(db->data + db->len, s, n);
    db->len += n;
    db->data[db->len] = '\0';
    return 1;
}

static int db_append_str(DynBuf *db, const char *s) {
    if (!s) return 1;
    return db_append_n(db, s, strlen(s));
}

static int db_append_char(DynBuf *db, char c) {
    if (!db_ensure(db, 1)) return 0;
    db->data[db->len++] = c;
    db->data[db->len] = '\0';
    return 1;
}

static void db_printf(DynBuf *db, const char *fmt, ...) {
    if (!db->data && db->cap == 0) {
        if (!db_init(db)) return;
    }
    va_list args;
    va_start(args, fmt);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed < 0) return;
    if (!db_ensure(db, (size_t)needed)) return;
    
    va_start(args, fmt);
    vsnprintf(db->data + db->len, (size_t)needed + 1, fmt, args);
    va_end(args);
    db->len += (size_t)needed;
}

static void db_free(DynBuf *db) {
    if (db->data) free(db->data);
}

/* Layout Logic */

static void layout_add_line(Document *doc, const char *line) {
    if (doc->rendered.count >= doc->rendered.capacity) {
        size_t new_cap = doc->rendered.capacity == 0 ? 10 : doc->rendered.capacity * 2;
        char **tmp = realloc(doc->rendered.lines, sizeof(char *) * new_cap);
        if (!tmp) return;
        doc->rendered.lines = tmp;
        doc->rendered.capacity = new_cap;
    }
    
    // Explicitly check if we have capacity now
    if (doc->rendered.count < doc->rendered.capacity) {
        char *s = strdup(line);
        if (s) doc->rendered.lines[doc->rendered.count++] = s;
    }
}

static int is_ansi(const char *p) {
    return (p[0] == '\x1b' && p[1] == '[');
}

static size_t ansi_len(const char *p) {
    const char *start = p;
    if (*p == '\x1b') p++;
    if (*p == '[') p++;
    while (*p && !((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))) p++;
    if (*p) p++;
    return p - start;
}

static int visible_len_raw(const char *text) {
    if (!text) return 0;
    int len = 0;
    const char *p = text;
    size_t remaining = strlen(text);
    mbstate_t state;
    memset(&state, 0, sizeof(state));

    while (*p) {
        if ((*p == '*' || *p == '_') && *(p+1) == *p) {
            p += 2;
            remaining -= 2;
        } else {
            wchar_t wc;
            size_t n = mbrtowc(&wc, p, remaining, &state);
            if (n == (size_t)-1 || n == (size_t)-2 || n == 0) {
                len++;
                p++;
                remaining--;
                memset(&state, 0, sizeof(state));
            } else {
                int w = wcwidth(wc);
                if (w > 0) len += w;
                p += n;
                remaining -= n;
            }
        }
    }
    return len;
}

static char *apply_formatting(const char *text) {
    if (!text) return strdup("");
    DynBuf db;
    if (!db_init(&db)) return strdup("");
    const char *p = text;
    int in_bold = 0;
    int in_underline = 0;

    while (*p) {
        if (*p == '*' && *(p+1) == '*') {
            in_bold = !in_bold;
            if (!db_append_str(&db, in_bold ? "\x1b[1m" : "\x1b[22m")) break;
            p += 2;
        } else if (*p == '_' && *(p+1) == '_') {
            in_underline = !in_underline;
            if (!db_append_str(&db, in_underline ? "\x1b[4m" : "\x1b[24m")) break;
            p += 2;
        } else {
            if (!db_append_char(&db, *p++)) break;
        }
    }
    db_append_str(&db, "\x1b[0m");
    char *res = db.data ? strdup(db.data) : strdup("");
    db_free(&db);
    return res;
}

static void layout_wrap_and_add(Document *doc, const char *text, int width, int left_pad) {
    char *formatted = apply_formatting(text);
    if (!formatted) return;
    
    const char *p = formatted;
    size_t remaining = strlen(formatted);
    int is_bold = 0;
    int is_under = 0;
    
    DynBuf db;
    if (!db_init(&db)) {
        free(formatted);
        return;
    }

    while (*p) {
        db_clear(&db);
        int vis = 0;

        for (int i = 0; i < left_pad; i++) db_append_char(&db, ' ');
        if (is_bold) db_append_str(&db, "\x1b[1m");
        if (is_under) db_append_str(&db, "\x1b[4m");

        const char *last_space_p = NULL;
        size_t last_space_remaining = 0;
        size_t last_space_db_len = 0;
        int last_space_bold = is_bold;
        int last_space_under = is_under;

        while (*p && vis < width) {
            if (is_ansi(p)) {
                size_t alen = ansi_len(p);
                if (strncmp(p, "\x1b[1m", 4) == 0) is_bold = 1;
                else if (strncmp(p, "\x1b[22m", 5) == 0) is_bold = 0;
                else if (strncmp(p, "\x1b[4m", 4) == 0) is_under = 1;
                else if (strncmp(p, "\x1b[24m", 5) == 0) is_under = 0;
                else if (strncmp(p, "\x1b[0m", 4) == 0) { is_bold = 0; is_under = 0; }

                if (!db_append_n(&db, p, alen)) {
                    p += alen;
                    remaining -= alen;
                    continue;
                }
                p += alen;
                remaining -= alen;
            } else {
                if (*p == ' ') {
                    last_space_p = p;
                    last_space_remaining = remaining;
                    last_space_db_len = db.len;
                    last_space_bold = is_bold;
                    last_space_under = is_under;
                }
                
                wchar_t wc;
                mbstate_t state;
                memset(&state, 0, sizeof(state));
                size_t n = mbrtowc(&wc, p, remaining, &state);
                if (n == (size_t)-1 || n == (size_t)-2 || n == 0) {
                    db_append_char(&db, *p++);
                    remaining--;
                    vis++;
                } else {
                    int w = wcwidth(wc);
                    if (w < 0) w = 1; // Fallback for non-printable
                    if (vis + w > width && vis > 0) break; // Don't exceed width
                    db_append_n(&db, p, n);
                    p += n;
                    remaining -= n;
                    vis += w;
                }
            }
        }

        if (*p && vis >= width && last_space_p) {
            db.len = last_space_db_len;
            db.data[db.len] = '\0';
            db_append_str(&db, "\x1b[0m");
            layout_add_line(doc, db.data);
            p = last_space_p + 1;
            remaining = last_space_remaining - 1;
            is_bold = last_space_bold;
            is_under = last_space_under;
        } else {
            db_append_str(&db, "\x1b[0m");
            layout_add_line(doc, db.data);
        }
    }
    db_free(&db);
    free(formatted);
}

void layout_build(Document *doc, int width) {
    layout_free(doc);
    if (width < 20) width = 20;

    int max_w = 80;

    // Text Padding: 10% Left, 10% Right
    int text_lp = (int)(width * 0.10);
    int text_rp = (int)(width * 0.10);
    int text_w = width - text_lp - text_rp;
    if (text_w > max_w) {
        text_w = max_w;
        text_lp = (width - text_w) / 2;
    }

    // DIV Padding: 8% Left, 10% Right
    int div_lp = (int)(width * 0.08);
    int div_rp = (int)(width * 0.10);
    int div_w_calc = width - div_lp - div_rp;
    if (div_w_calc > max_w) {
        // Keep the 2% left offset relative to text for hierarchy
        div_lp = text_lp - (int)(width * 0.02);
        if (div_lp < 1) div_lp = 1;
    }

    for (size_t i = 0; i < doc->element_count; i++) {
        Line *el = &doc->elements[i];
        
        switch (el->type) {
            case LINE_TITLE: {
                int vis = visible_len_raw(el->content);
                int pad = (width - vis) / 2;
                if (pad < 0) pad = 0;
                
                DynBuf db;
                if (!db_init(&db)) break;
                for (int j = 0; j < pad; j++) db_append_char(&db, ' ');
                char *f = apply_formatting(el->content);
                db_printf(&db, "\x1b[1m\x1b[4m%s\x1b[0m", f);
                free(f);
                layout_add_line(doc, db.data);
                db_free(&db);
                
                layout_add_line(doc, "");
                break;
            }
            case LINE_DIV: {
                if (doc->rendered.count > 0) layout_add_line(doc, "");
                
                DynBuf db;
                if (!db_init(&db)) break;
                for (int j = 0; j < div_lp; j++) db_append_char(&db, ' ');
                char *f = apply_formatting(el->content);
                db_printf(&db, "\x1b[1m%s\x1b[0m", f);
                free(f);
                layout_add_line(doc, db.data);
                db_free(&db);
                break;
            }
            case LINE_SPACE: {
                layout_add_line(doc, "");
                break;
            }
            case LINE_TEXT: {
                layout_wrap_and_add(doc, el->content, text_w, text_lp);
                break;
            }
            default: break;
        }
    }

    // Add safe-area padding at the very end
    layout_add_line(doc, "");
}

void layout_free(Document *doc) {
    if (doc->rendered.lines) {
        for (size_t i = 0; i < doc->rendered.count; i++) {
            free(doc->rendered.lines[i]);
        }
        free(doc->rendered.lines);
    }
    doc->rendered.lines = NULL;
    doc->rendered.count = 0;
    doc->rendered.capacity = 0;
}
