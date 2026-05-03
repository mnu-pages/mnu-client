#include "mnu.h"
#include <curl/curl.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) return 0;

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

char *http_fetch(const char *category, const char *page, int *error_code) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    if (error_code) *error_code = 0;

    chunk.memory = malloc(1);
    if (!chunk.memory) return NULL;
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    const char *fmt = "%s/%s/%s.mn";
    int needed = snprintf(NULL, 0, fmt, MNU_BASE_URL, category, page);
    char *url = malloc((size_t)needed + 1);
    if (!url) {
        free(chunk.memory);
        curl_easy_cleanup(curl_handle);
        return NULL;
    }
    snprintf(url, (size_t)needed + 1, fmt, MNU_BASE_URL, category, page);

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; mnu-client/1.0)");
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl_handle);

    char *result = NULL;

    if (res != CURLE_OK) {
        if (error_code) *error_code = -1; // Network error
        free(chunk.memory);
    } else {
        long response_code;
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            result = chunk.memory;
        } else {
            if (error_code) *error_code = (int)response_code;
            free(chunk.memory);
        }
    }

    free(url);
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    return result;
}
