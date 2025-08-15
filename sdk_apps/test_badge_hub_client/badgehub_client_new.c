#include "badgehub_client_new.h"

#include "cJSON.h"
#include "stdio.h"
#include "utils.h"

#include <stdlib.h>

#include <string.h>

#define HTTP_USERAGENT           "BadgeHubClient/1.0"
#define BADGEHUB_BASE_URL        "https://badge.why2025.org/api/v3"
#define BADGEHUB_PROJECT_DETAIL  BADGEHUB_BASE_URL "/projects/%s"
#define BADGEHUB_LATEST_REVISION BADGEHUB_BASE_URL "/project-latest-revisions/%s"
#define BADGEHUB_REVISION_FILE   BADGEHUB_BASE_URL "/projects/%s/rev%i/files/%s"
#define BADGEHUB_REVISION        BADGEHUB_BASE_URL "/projects/%s/rev%i"
#define BADGEHUB_PING            BADGEHUB_BASE_URL "/ping?id=%s-v1&mac=%s"
#define BADGEHUB_SEARCH          BADGEHUB_BASE_URL "/project-summaries?search=%s&pageLength=%d&pageStart=%d"

project_t *get_projects(char const *search_query, int limit, int offset, int *out_num) {
    http_data_t http_response;
    char       *url          = NULL;
    cJSON      *project_json = NULL;
    project_t  *result       = NULL;

    asprintf(&url, BADGEHUB_SEARCH, search_query, limit, offset);
    if (!do_http(url, &http_response, NULL)) {
        printf("Failed searching for '%s'\n", search_query);
        goto out;
    }

    project_json = cJSON_Parse(http_response.memory);
    if (!project_json) {
        char const *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error: JSON parse error before: %s\n", error_ptr);
        }
        goto out;
    }

    {
        cJSON *project = NULL;
        int    i       = 0;
        *out_num       = cJSON_GetArraySize(project_json);
        result         = calloc(*out_num, sizeof(project_t));
        cJSON_ArrayForEach(project, project_json) {
            if (!cJSON_IsObject(project)) {
                printf("Warning: Skipping non-object item in files array\n");
                continue;
            }
            // printf("%s\n", cJSON_Print(project));
            result[i].name        = get_json_string(project, "name");
            result[i].slug        = get_json_string(project, "slug");
            result[i].description = get_json_string(project, "description");
            result[i].project_url = get_json_string(project, "project_url");

            cJSON *icon_map    = cJSON_GetObjectItemCaseSensitive(project_json, "icon_map");
            cJSON *icon_64_obj = cJSON_GetObjectItemCaseSensitive(icon_map, "64x64");
            result[i].icon_url = get_json_string(icon_64_obj, "url");

            cJSON *revision_item = cJSON_GetObjectItemCaseSensitive(project_json, "revision");
            result[i].revision   = cJSON_IsNumber(revision_item) ? revision_item->valueint : 0;
            i++;
        }
        cJSON_free(project);
    }

out:
    free(url);
    return result;
}

int get_projects_files(char const *slug, int revision, project_file_t *out_project_files) {
    http_data_t http_response;
    char       *url          = NULL;
    cJSON      *project_json = NULL;
    int         result       = -1;

    // TODO
    asprintf(&url, BADGEHUB_REVISION_FILE, slug, revision);
    if (!do_http(url, &http_response, NULL)) {
        printf("Failed searching for '%s'\n", slug);
        goto out;
    }

    project_json = cJSON_Parse(http_response.memory);
    if (!project_json) {
        char const *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error: JSON parse error before: %s\n", error_ptr);
        }
        goto out;
    }
out:
    free(url);
    return result;
}

project_detail_t *get_project_details(char const *slug, int revision) {
    http_data_t       http_response;
    char             *url                 = NULL;
    cJSON            *project_detail_json = NULL;
    cJSON            *version_json        = NULL;
    cJSON            *metadata_json       = NULL;
    cJSON            *file_json           = NULL;
    project_detail_t *result              = malloc(sizeof(project_detail_t));

    if (revision) {
        asprintf(&url, BADGEHUB_REVISION, slug, revision);
        result->revision = revision;
    } else {
        asprintf(&url, BADGEHUB_PROJECT_DETAIL, slug);
    }

    if (!do_http(url, &http_response, NULL)) {
        printf("Failed searching for '%s'\n", slug);
        goto out;
    }

    project_detail_json = cJSON_Parse(http_response.memory);
    if (!project_detail_json) {
        char const *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error: JSON parse error before: %s\n", error_ptr);
        }
        goto out;
    }

    version_json = cJSON_GetObjectItemCaseSensitive(project_detail_json, "version");

    if (!version_json || !cJSON_IsObject(version_json)) {
        printf("No version object in JSON: %s\n", project_detail_json->valuestring);
        goto out;
    }

    metadata_json = cJSON_GetObjectItemCaseSensitive(version_json, "app_metadata");
    file_json     = cJSON_GetObjectItemCaseSensitive(version_json, "files");

    if (!metadata_json || !cJSON_IsObject(metadata_json)) {
        printf("No metadata object in JSON: %s\n", version_json->valuestring);
        goto out;
    }
    if (!metadata_json || !cJSON_IsArray(file_json)) {
        printf("No file array in JSON: %s\n", version_json->valuestring);
        goto out;
    }
    if (!revision) {
        cJSON *revision_item = cJSON_GetObjectItemCaseSensitive(version_json, "revision");
        result->revision     = cJSON_IsNumber(revision_item) ? revision_item->valueint : 0;
    }

    result->name         = get_json_string(metadata_json, "name");
    result->description  = get_json_string(metadata_json, "description");
    result->author       = get_json_string(metadata_json, "author");
    result->version      = get_json_string(metadata_json, "version");
    result->published_at = get_json_string(version_json, "published_at");
    result->slug         = strdup(slug);
    result->file_count   = cJSON_GetArraySize(file_json);
    result->files        = calloc(result->file_count, sizeof(project_file_t));

    {
        cJSON *singe_file = NULL;
        int    i          = 0;
        cJSON_ArrayForEach(singe_file, file_json) {
            result->files[i].full_path = get_json_string(singe_file, "full_path");
            result->files[i].sha256    = get_json_string(singe_file, "sha256");
            result->files[i].url       = get_json_string(singe_file, "url");
            i++;
        }
        cJSON_free(singe_file);
    }

out:
    free(url);
    cJSON_free(file_json);
    cJSON_free(metadata_json);
    cJSON_free(version_json);
    cJSON_free(project_detail_json);
    return result;
}
void free_project_details(project_detail_t *details) {
    if (!details)
        return;
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
void free_projects(project_t *details, int count) {

    if (!details)
        return;
    for (int i = 0; i < count; i++) {
        free(details[i].name);
        free(details[i].slug);
        free(details[i].description);
        free(details[i].project_url);
        free(details[i].icon_url);
    }
    free(details);
};
