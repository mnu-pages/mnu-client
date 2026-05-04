#include "mnu.h"
#include <curl/curl.h>
#include <sys/utsname.h>
#include <unistd.h>

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

static unsigned int fnv1a_hash_32(const char *str) {
    unsigned int hash = 0x811c9dc5;
    while (*str) {
        hash ^= (unsigned char)*str++;
        hash *= 0x01000193;
    }
    return hash;
}

static void get_anonymous_id(char *out, size_t max) {
    struct utsname un;
    if (uname(&un) == -1) {
        snprintf(out, max, "00000000");
        return;
    }
    
    char seed[1024];
    // Added a static salt for hardening and combined with system info
    const char *salt = "mnu-v0.2.2-salt";
    snprintf(seed, sizeof(seed), "%s-%s-%s-%s-%u", salt, un.sysname, un.nodename, un.machine, (unsigned int)getuid());
    
    unsigned int h = fnv1a_hash_32(seed);
    snprintf(out, max, "%08x", h);
}

char *http_fetch(const char *category, const char *page, int *error_code) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    static char anon_id[9] = {0};

    if (error_code) *error_code = 0;

    if (anon_id[0] == '\0') {
        get_anonymous_id(anon_id, sizeof(anon_id));
    }

    chunk.memory = malloc(1);
    if (!chunk.memory) return NULL;
    chunk.size = 0;

    curl_handle = curl_easy_init();
    if (!curl_handle) {
        free(chunk.memory);
        return NULL;
    }

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
    
    char ua[128];
    snprintf(ua, sizeof(ua), "Mozilla/5.0 (compatible; mnu-client/%s; ID/%s)", MNU_VERSION, anon_id);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, ua);
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

    return result;
}
