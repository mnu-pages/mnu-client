#include "mnu.h"
#include <string.h>
#include <ctype.h>

static char *trim_and_copy(const char *start, const char *end) {
    while (start < end && isspace((unsigned char)*start)) start++;
    while (end > start && isspace((unsigned char)end[-1])) end--;
    size_t len = (size_t)(end - start);
    char *res = malloc(len + 1);
    if (res) {
        memcpy(res, start, len);
        res[len] = '\0';
    }
    return res;
}

static char *extract_quoted(const char *line) {
    const char *start = strchr(line, '"');
    if (!start) return NULL;
    const char *end = strrchr(line, '"');
    if (end <= start) return NULL;
    return trim_and_copy(start + 1, end);
}

Document *parser_parse(const char *raw_data, const char *category, const char *page) {
    if (!raw_data) return NULL;

    Document *doc = malloc(sizeof(Document));
    if (!doc) return NULL;

    doc->category = strdup(category);
    doc->page = strdup(page);
    doc->element_count = 0;
    doc->element_capacity = 32;
    doc->elements = malloc(sizeof(Line) * doc->element_capacity);
    doc->rendered.lines = NULL;
    doc->rendered.count = 0;
    doc->rendered.capacity = 0;

    const char *current = raw_data;
    int last_was_space = 0;

    while (*current) {
        const char *line_end = strchr(current, '\n');
        size_t line_len;
        if (line_end) {
            line_len = (size_t)(line_end - current);
        } else {
            line_len = strlen(current);
        }

        // Temporary line buffer for easier processing
        char *line_tmp = malloc(line_len + 1);
        if (!line_tmp) break;
        memcpy(line_tmp, current, line_len);
        line_tmp[line_len] = '\0';

        // Trim leading space for directive check
        char *trimmed = line_tmp;
        while (*trimmed && isspace((unsigned char)*trimmed)) trimmed++;

        LineType type = LINE_TEXT;
        char *content = NULL;

        if (strncmp(trimmed, ".TITLE", 6) == 0) {
            content = extract_quoted(trimmed);
            if (content) {
                type = LINE_TITLE;
            } else {
                type = LINE_TEXT;
                content = strdup(trimmed);
            }
        } else if (strncmp(trimmed, ".DIV", 4) == 0) {
            content = extract_quoted(trimmed);
            if (content) {
                type = LINE_DIV;
            } else {
                type = LINE_TEXT;
                content = strdup(trimmed);
            }
        } else if (*trimmed == '\0') {
            type = LINE_SPACE;
            content = strdup("");
        } else {
            type = LINE_TEXT;
            content = strdup(trimmed);
        }

        if (!content) {
            free(line_tmp);
            break;
        }

        // Optimization: Collapse consecutive spaces
        if (type == LINE_SPACE) {
            if (last_was_space || doc->element_count == 0) {
                free(line_tmp);
                if (line_end) current = line_end + 1; else break;
                continue;
            }
            last_was_space = 1;
        } else {
            last_was_space = 0;
        }

        if (doc->element_count >= doc->element_capacity) {
            size_t new_cap = doc->element_capacity * 2;
            Line *tmp = realloc(doc->elements, sizeof(Line) * new_cap);
            if (!tmp) {
                free(content);
                free(line_tmp);
                break;
            }
            doc->elements = tmp;
            doc->element_capacity = new_cap;
        }

        doc->elements[doc->element_count].type = type;
        doc->elements[doc->element_count].content = content;
        doc->element_count++;

        free(line_tmp);
        if (line_end) current = line_end + 1; else break;
    }

    return doc;
}

void parser_free(Document *doc) {
    if (!doc) return;
    free(doc->category);
    free(doc->page);
    for (size_t i = 0; i < doc->element_count; i++) {
        free(doc->elements[i].content);
    }
    free(doc->elements);
    layout_free(doc);
    free(doc);
}
