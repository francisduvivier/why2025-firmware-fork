#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdio.h>  // For FILE
#include <stddef.h> // For size_t
#include "cJSON.h" // For cJSON

// Struct to help curl write data into a dynamically growing memory buffer
struct MemoryStruct {
    char *memory;
    size_t size;
};

typedef struct http_data {
    char  *memory;
    size_t size;
} http_data_t;

typedef struct http_file {
    FILE  *fp;
    size_t size;
} http_file_t;

// libcurl callback function to write received data into our MemoryStruct
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

// libcurl callback to write downloaded file data to a FILE* handle
size_t WriteFileCallback(void *ptr, size_t size, size_t nmemb, FILE *stream);

// Helper function to safely extract a string from a cJSON object.
char* get_json_string(cJSON *json, const char *key);


bool do_http(char const *url, http_data_t *response_data, http_file_t *http_file);

#endif // UTILS_H
