#include "mnu.h"
#include <stdio.h>
#include <stdlib.h>

char *runner_load_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = malloc(fsize + 1);
    if (data) {
        size_t read_bytes = fread(data, 1, fsize, f);
        data[read_bytes] = '\0';
    }
    
    fclose(f);
    return data;
}
