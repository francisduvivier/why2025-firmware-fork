#include "utils.h"

#include "cJSON.h"

#include <stdlib.h>

#include <curl/curl.h>
#include <string.h>
#include <sys/stat.h>

// libcurl callback function to write received data into our MemoryStruct
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t               realsize = size * nmemb;
    struct MemoryStruct *mem      = (struct MemoryStruct *)userp;
    char                *ptr      = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size              += realsize;
    mem->memory[mem->size]  = 0;
    return realsize;
}

// libcurl callback to write downloaded file data to a FILE* handle
size_t WriteFileCallback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

// Helper function to safely extract a string from a cJSON object.
char *get_json_string(cJSON *json, char const *key) {
    if (!json)
        return NULL;
    cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
    if (cJSON_IsString(item) && (item->valuestring != NULL)) {
        char *str = malloc(strlen(item->valuestring) + 1);
        if (str) {
            strcpy(str, item->valuestring);
        } else {
            printf("Malloc failed\n");
            return NULL;
        }
        return str;
    } else {
        printf("Error getting key '%s' from JSON %s\n", key, json->valuestring);
        return NULL;
    }
}
static size_t mem_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    // printf("mem_cb(%p, %zu, %zu, %p)\n", contents, size, nmemb, userp);

    size_t       realsize = size * nmemb;
    http_data_t *mem      = (http_data_t *)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        printf("Malloc failed\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size              += realsize;
    mem->memory[mem->size]  = 0;

    return realsize;
}

static size_t file_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    // printf("file_cb(%p, %zu, %zu, %p)\n", contents, size, nmemb, userp);

    size_t       realsize = size * nmemb;
    http_file_t *file     = (http_file_t *)userp;
    FILE        *f        = file->fp;

    size_t s = fwrite(contents, 1, size * nmemb, f);
    if (s != size * nmemb) {
        printf("Failure writing to file\n");
        return 0;
    }

    file->size += realsize;
    return realsize;
}

bool do_http(char const *url, http_data_t *response_data, http_file_t *http_file) {
    // printf("do_http(%s, %p, %p)\n", url, response_data, http_file);

    bool ret = false;

    if (!response_data && !http_file) {
        printf("No response data pointer provided\n");
        return false;
    }

    if (!url) {
        printf("No URL provided\n");
        return false;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("Failed to allocate curl\n");
        return false;
    }

    if (response_data) {
        memset(response_data, 0, sizeof(http_data_t));
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    if (response_data) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mem_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response_data);
    } else {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, file_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)http_file);
    }

    // curl_easy_setopt(curl, CURLOPT_USERAGENT, HTTP_USERAGENT);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        char const *error_string = curl_easy_strerror(res);
        if (error_string) {
            printf("do_http(%s) curl_easy_perform() failed: %s\n", url, curl_easy_strerror(res));
        } else {
            printf("do_http(%s) curl_easy_perform() failed: %u\n", url, res);
        }
        goto out;
    }

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    if (response_data) {
        printf("do_http(%s) response code: %ld, bytes received: %zu\n", url, response_code, response_data->size);
        response_data->memory                      = realloc(response_data->memory, response_data->size + 1);
        response_data->memory[response_data->size] = 0;
    } else {
        printf("do_http(%s) response code: %ld, bytes received: %zu\n", url, response_code, http_file->size);
    }

    if (response_code < 200 || response_code > 299) {
        if (response_data && response_data->size) {
            printf("do_http(%s) error: server response: '%s'\n", url, (char *)response_data->memory);
        } else {
            printf("do_http(%s) error: server response: <no response>\n", url);
        }
        goto out;
    }

    ret = true;
out:
    curl_easy_cleanup(curl);
    printf("do_http(%s) returned %i\n", url, ret);
    return ret;
}
