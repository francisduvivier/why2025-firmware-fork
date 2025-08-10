#include "badgehub_client.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"
#include <sys/stat.h>
// #include <badgevms/application.h>
// #include <badgevms/process.h>
// #include <badgevms/wifi.h>

#define INSTALLATION_DIR "installation_dir"


// TODO
int escape_string(const char* str, int bytes, char* escaped_str )
{
  for (int i = 0; i <= bytes; i++) {
    if (str[i]==22) {
          
    }
  }
}

project_t *get_applications(int *project_count, const char* search_query, int limit, int offset) {
    printf("Init curl\n");
    *project_count = 0;
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk = { .memory = malloc(1), .size = 0 };
    project_t *projects = NULL;
    char url[512];
    char base_url[256];
    snprintf(base_url, sizeof(base_url), "%s/project-summaries", "https://badge.why2025.org/api/v3");
    curl_global_init(1);
    curl_handle = curl_easy_init();
    if (!curl_handle) { free(chunk.memory); return NULL; }
    if (search_query && strlen(search_query) > 0) {
        // char *escaped_query = NULL;
        // curl_easy_perform
        // escaped_query = curl_easy_escape(curl_handle, search_query, 0);
        const char *escaped_query = search_query;
        printf("Searching with ?search=%s&pageLength=%d&pageStart=%d\n", escaped_query, limit, offset);
        snprintf(url, sizeof(url), "%s?search=%s&pageLength=%d&pageStart=%d", base_url, escaped_query, limit, offset);
        curl_easy_cleanup(escaped_query);
    } else {
        snprintf(url, sizeof(url), "%s?pageLength=%d&pageStart=%d", base_url, limit, offset);
        printf("Searching all\n");
    }
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "lvgl-badgehub-client/1.0");
    res = curl_easy_perform(curl_handle);
    if (res == CURLE_OK) {
        cJSON *root = cJSON_Parse(chunk.memory);
        if (cJSON_IsArray(root)) {
            *project_count = cJSON_GetArraySize(root);
            projects = calloc(*project_count, sizeof(project_t));
            if (projects) {
                cJSON *proj_json = NULL;
                int i = 0;
                cJSON_ArrayForEach(proj_json, root) {
                    projects[i].name = get_json_string(proj_json, "name");
                    projects[i].slug = get_json_string(proj_json, "slug");
                    projects[i].description = get_json_string(proj_json, "description");
                    projects[i].project_url = get_json_string(proj_json, "project_url");

                    cJSON *icon_map = cJSON_GetObjectItemCaseSensitive(proj_json, "icon_map");
                    cJSON *icon_64_obj = cJSON_GetObjectItemCaseSensitive(icon_map, "64x64");
                    projects[i].icon_url = get_json_string(icon_64_obj, "url");

                    cJSON *revision_item = cJSON_GetObjectItemCaseSensitive(proj_json, "revision");
                    projects[i].revision = cJSON_IsNumber(revision_item) ? revision_item->valueint : 0;
                    i++;
                }
            }
        }
        cJSON_Delete(root);
    } else {
      return NULL;
    }
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();
    return projects;
}

uint8_t* download_icon_to_memory(const char* icon_url, int* data_size) {
    if (!icon_url || strlen(icon_url) == 0) return NULL;

    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk = { .memory = malloc(1), .size = 0 };
    long response_code = 0;

    curl_handle = curl_easy_init();
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, icon_url);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        res = curl_easy_perform(curl_handle);
        if (res == CURLE_OK) {
            curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code == 200) {
                *data_size = chunk.size;
                curl_easy_cleanup(curl_handle);
                return (uint8_t*)chunk.memory;
            }
        }
        free(chunk.memory);
        curl_easy_cleanup(curl_handle);
    }
    return NULL;
}

void free_applications(project_t *projects, int count) {
    if (!projects) return;
    for (int i = 0; i < count; i++) {
        free(projects[i].name);
        free(projects[i].slug);
        free(projects[i].description);
        free(projects[i].project_url);
        free(projects[i].icon_url);
    }
    free(projects);
}

// ... (rest of badgehub_client.c is unchanged)
project_detail_t *get_project_details(const char *slug, int revision) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk = { .memory = malloc(1), .size = 0 };
    project_detail_t *details = NULL;
    char url[256];
    if (!slug) return NULL;
    snprintf(url, sizeof(url), "%s/projects/%s/rev%d", "https://badge.why2025.org/api/v3", slug, revision);
    if (chunk.memory == NULL) return NULL;
    curl_global_init(0);
    curl_handle = curl_easy_init();
    if (!curl_handle) { free(chunk.memory); return NULL; }
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "lvgl-badgehub-client/1.0");
    res = curl_easy_perform(curl_handle);
    if (res == CURLE_OK) {
        cJSON *root = cJSON_Parse(chunk.memory);
        if (root) {
            details = calloc(1, sizeof(project_detail_t));
            if (details) {
                details->slug = strdup(slug);
                details->revision = revision;
                cJSON *version_obj = cJSON_GetObjectItemCaseSensitive(root, "version");
                if (version_obj) {
                    cJSON *metadata_obj = cJSON_GetObjectItemCaseSensitive(version_obj, "app_metadata");
                    details->name = get_json_string(metadata_obj, "name");
                    details->description = get_json_string(metadata_obj, "description");
                    details->author = get_json_string(metadata_obj, "author");
                    details->version = get_json_string(metadata_obj, "version");
                    details->published_at = get_json_string(version_obj, "published_at");
                    cJSON *files_array = cJSON_GetObjectItemCaseSensitive(version_obj, "files");
                    if (cJSON_IsArray(files_array)) {
                        details->file_count = cJSON_GetArraySize(files_array);
                        details->files = malloc(details->file_count * sizeof(project_file_t));
                        if (details->files) {
                            cJSON *file_json = NULL;
                            int i = 0;
                            cJSON_ArrayForEach(file_json, files_array) {
                                details->files[i].full_path = get_json_string(file_json, "full_path");
                                details->files[i].sha256 = get_json_string(file_json, "sha256");
                                details->files[i].url = get_json_string(file_json, "url");
                                i++;
                            }
                        }
                    }
                }
            }
            cJSON_Delete(root);
        }
    }
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();
    return details;
}
bool download_project_file(const project_file_t* file_info, const char* project_slug) {
    if (!file_info || !file_info->url || !project_slug) return false;
    CURL *curl_handle;
    CURLcode res;
    FILE *fp;
    char local_path[512];
    long response_code = 0;
    bool success = false;
    snprintf(local_path, sizeof(local_path), "%s/%s/%s", INSTALLATION_DIR, project_slug, file_info->full_path);
    ensure_dir_exists(local_path);
    curl_handle = curl_easy_init();
    if (curl_handle) {
        fp = fopen(local_path, "wb");
        if (fp) {
            curl_easy_setopt(curl_handle, CURLOPT_URL, file_info->url);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteFileCallback);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, fp);
            res = curl_easy_perform(curl_handle);
            fclose(fp);
            if (res == CURLE_OK) {
                curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
                if (response_code == 200) {
                    success = true;
                } else {
                    fprintf(stderr, "Download failed for %s: HTTP status %ld\n", file_info->url, response_code);
                    remove(local_path);
                }
            } else {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
        } else {
            fprintf(stderr, "Failed to open file for writing: %s\n", local_path);
        }
        curl_easy_cleanup(curl_handle);
    }
    return success;
}
void free_project_details(project_detail_t *details) {
    if (!details) return;
    free(details->name);
    free(details->description);
    free(details->published_at);
    free(details->author);
    free(details->version);
    free(details->slug);
    if (details->files) {
        for (int i = 0; i < details->file_count; i++) {
            free(details->files[i].full_path);
            free(details->files[i].sha256);
            free(details->files[i].url);
        }
        free(details->files);
    }
    free(details);
}
//
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
    printf("do_http(%s, %p, %p)\n", url, response_data, http_file);

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
//
// bool update_application_file(application_t *app, char const *relative_file_name, char const *file_url) {
//       bool  ret                = false;
//       char *absolute_file_name = NULL;
//       char *tmpfile            = NULL;
//       FILE *f                  = NULL;
//
//       absolute_file_name = application_create_file_string(app, relative_file_name);
//     if (!absolute_file_name) {
//         printf("Illegal file name %s\n", relative_file_name);
//         goto out;
//     }
//
//     asprintf(&tmpfile, "%s.inst", absolute_file_name);
//     if (!tmpfile) {
//         printf("Unable to allocate tmpfilename\n");
//         goto out;
//     }
//
//     f = fopen(tmpfile, "w");
//     if (!f) {
//         printf("Unable to open tmpfile %s\n", tmpfile);
//         goto out;
//     }
//
//     http_file_t file_op;
//       file_op.size = 0;
//     file_op.fp   = f;
//
//     if (!do_http(file_url, NULL, &file_op)) {
//         printf("Unable to write save tmpfile %s\n", tmpfile);
//         goto out;
//     }
//
//     fclose(f);
//     f = NULL;
//
//     remove(absolute_file_name);
//
//     if (rename(tmpfile, absolute_file_name)) {
//         printf("Unable to move tmpfile to final %s -> %s\n", tmpfile, absolute_file_name);
//         goto out;
//     }
//
//     ret = true;
//
// out:
//     free(tmpfile);
//     free(absolute_file_name);
//     if (f) {
//         fclose(f);
//     }
//     return ret;
// }
//
