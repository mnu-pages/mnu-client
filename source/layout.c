#include "mnu.h"
#include <string.h>
#include <stdarg.h>

/* Dynamic Buffer Utility */
typedef struct {
    char *data;
    size_t len;
    size_t cap;
} DynBuf;

static void db_init(DynBuf *db) {
    db->cap = 128;
    db->data = malloc(db->cap);
    db->data[0] = '\0';
    db->len = 0;
}

static void db_ensure(DynBuf *db, size_t add) {
    if (db->len + add >= db->cap) {
        while (db->len + add >= db->cap) db->cap *= 2;
        char *tmp = realloc(db->data, db->cap);
        if (tmp) {
            db->data = tmp;
        }
    }
}

static void db_append_str(DynBuf *db, const char *s) {
    if (!s) return;
    size_t slen = strlen(s);
    db_ensure(db, slen);
    memcpy(db->data + db->len, s, slen);
    db->len += slen;
    db->data[db->len] = '\0';
}

static void db_append_char(DynBuf *db, char c) {
    db_ensure(db, 1);
    db->data[db->len++] = c;
    db->data[db->len] = '\0';
}

static void db_printf(DynBuf *db, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed < 0) return;
    db_ensure(db, (size_t)needed);
    
    va_start(args, fmt);
    vsnprintf(db->data + db->len, (size_t)needed + 1, fmt, args);
    va_end(args);
    db->len += (size_t)needed;
}

static void db_free(DynBuf *db) {
    free(db->data);
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
    char *s = strdup(line);
    if (s) doc->rendered.lines[doc->rendered.count++] = s;
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
    while (*p) {
        if ((*p == '*' || *p == '_') && *(p+1) == *p) {
            p += 2;
        } else {
            len++;
            p++;
        }
    }
    return len;
}

static char *apply_formatting(const char *text) {
    if (!text) return strdup("");
    DynBuf db;
    db_init(&db);
    const char *p = text;
    int in_bold = 0;
    int in_underline = 0;

    while (*p) {
        if (*p == '*' && *(p+1) == '*') {
            in_bold = !in_bold;
            db_append_str(&db, in_bold ? "\x1b[1m" : "\x1b[22m");
            p += 2;
        } else if (*p == '_' && *(p+1) == '_') {
            in_underline = !in_underline;
            db_append_str(&db, in_underline ? "\x1b[4m" : "\x1b[24m");
            p += 2;
        } else {
            db_append_char(&db, *p++);
        }
    }
    db_append_str(&db, "\x1b[0m");
    char *res = strdup(db.data);
    db_free(&db);
    return res;
}

static void layout_wrap_and_add(Document *doc, const char *text, int width, int left_pad) {
    char *formatted = apply_formatting(text);
    const char *p = formatted;
    int is_bold = 0;
    int is_under = 0;
    
    while (*p) {
        DynBuf db;
        db_init(&db);
        int vis = 0;

        for (int i = 0; i < left_pad; i++) db_append_char(&db, ' ');
        if (is_bold) db_append_str(&db, "\x1b[1m");
        if (is_under) db_append_str(&db, "\x1b[4m");

        const char *last_space_p = NULL;
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

                db_ensure(&db, alen);
                memcpy(db.data + db.len, p, alen);
                db.len += alen;
                db.data[db.len] = '\0';
                p += alen;
            } else {
                if (*p == ' ') {
                    last_space_p = p;
                    last_space_db_len = db.len;
                    last_space_bold = is_bold;
                    last_space_under = is_under;
                }
                db_append_char(&db, *p++);
                vis++;
            }
        }

        if (*p && vis >= width && last_space_p) {
            db.len = last_space_db_len;
            db.data[db.len] = '\0';
            db_append_str(&db, "\x1b[0m");
            layout_add_line(doc, db.data);
            p = last_space_p + 1;
            is_bold = last_space_bold;
            is_under = last_space_under;
        } else {
            db_append_str(&db, "\x1b[0m");
            layout_add_line(doc, db.data);
        }
        db_free(&db);
    }
    free(formatted);
}

void layout_build(Document *doc, int width) {
    layout_free(doc);
    if (width < 20) width = 20;

    // Standardize readable width (80 chars is the industry standard for reading)
    int max_content_w = 80;
    int content_w = (int)(width * 0.85); // Use 85% of screen
    if (content_w > max_content_w) content_w = max_content_w;
    if (content_w < 15) content_w = width - 4; // Fallback for very small screens

    int left_pad = (width - content_w) / 2;

    for (size_t i = 0; i < doc->element_count; i++) {
        Line *el = &doc->elements[i];
        
        switch (el->type) {
            case LINE_TITLE: {
                int vis = visible_len_raw(el->content);
                int pad = (width - vis) / 2;
                if (pad < 0) pad = 0;
                
                DynBuf db;
                db_init(&db);
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
                db_init(&db);
                // Headers are slightly less indented than text for visual hierarchy
                int div_pad = left_pad - 2;
                if (div_pad < 2) div_pad = 2;
                
                for (int j = 0; j < div_pad; j++) db_append_char(&db, ' ');
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
                layout_wrap_and_add(doc, el->content, content_w, left_pad);
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
