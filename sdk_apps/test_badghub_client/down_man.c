#include "down_man.h"

#include "badgehub_client.h"
#include "badgevms/application.h"
#include "badgevms/process.h"
#include "cJSON.h"

#include <stdbool.h>

#include <string.h>

bool install_application_file(project_detail_t *p, int *out_got_files) {
    //, char const *relative_file_name, char const *file_url) {
    bool  ret, update = false;
    char *absolute_file_name = NULL;
    char *tmpfile            = NULL;
    FILE *f                  = NULL;


    application_t *app = application_create(p->slug, p->slug, p->author, p->version, "", APPLICATION_SOURCE_BADGEHUB);

    if (app) {
        printf("Created app %s\n", p->slug);
        update = true;
    } else {
        app = application_get(p->slug);
        // if (strcmp(app->version, p->version) != 0) {
            update = true;
        // }
    }
    if (update) {
        for (int i = 0; i < p->file_count; i++) {


            absolute_file_name = application_create_file_string(app, p->files->full_path);
            printf("Application file name %s\n", absolute_file_name);
            if (!absolute_file_name) {
                printf("Illegal file name %s\n", p->files->full_path);
                goto out;
            }

            asprintf(&tmpfile, "%s.inst", absolute_file_name);
            if (!tmpfile) {
                printf("Unable to allocate tmpfilename\n");
                goto out;
            }

            f = fopen(tmpfile, "w");
            if (!f) {
                printf("Unable to open tmpfile %s\n", tmpfile);
                goto out;
            }

            http_file_t file_op;
            file_op.size = 0;
            file_op.fp   = f;

            if (!do_http(p->files->url, NULL, &file_op)) {
                printf("Unable to write save tmpfile %s\n", tmpfile);
                goto out;
            }

            fclose(f);
            f = NULL;

            remove(absolute_file_name);

            if (rename(tmpfile, absolute_file_name)) {
                printf("Unable to move tmpfile to final %s -> %s\n", tmpfile, absolute_file_name);
                goto out;
            }

            ret = true;
            (*out_got_files)++;
        }
    }

    application_list_handle h = application_list(NULL);
    application_t          *a = application_list_get_next(h);
    printf("Installed Apps:\n");
    while (a) {
        printf("Application: %s (%s)\n", a->name, a->binary_path);
        if (!a->binary_path) {
            application_destroy(a);
        }
        a = application_list_get_next(h);
    }
    application_free(a);

    printf("Starting %s (%s)\n", p->slug, absolute_file_name);
    int pid = process_create(absolute_file_name, 8192, 0, NULL);
    if (pid == -1) {
        printf("Failed to start %s (%s)\n", p->slug, absolute_file_name);
    }



out:
    free(tmpfile);
    free(absolute_file_name);
    if (f) {
        fclose(f);
    }
    return ret;
}


bool install_application(application_t *app, char const *version) {
    FILE       *file             = NULL;
    char       *application_data = NULL;
    char       *url              = NULL;
    http_data_t response_data;

    cJSON *json             = NULL;
    cJSON *files_array      = NULL;
    cJSON *file_item        = NULL;
    cJSON *app_version      = NULL;
    cJSON *app_metadata     = NULL;
    cJSON *app_application  = NULL;
    cJSON *name_field       = NULL;
    cJSON *executable_field = NULL;
    char  *name             = NULL;
    char  *executable       = NULL;
    long   file_size        = 0;
    bool   result           = false;

    int revision = get_project_latest_revision(app->unique_identifier);
    if (revision < 0) {
        printf("Failed to get project revision\n");
        goto out;
    }

    asprintf(&url, BADGEHUB_REVISION, app->unique_identifier, revision);
    if (!do_http(url, &response_data, NULL)) {
        printf("Failed to read project revision data\n");
        goto out;
    }

    json = cJSON_Parse(response_data.memory);
    if (!json) {
        char const *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error: JSON parse error before: %s\n", error_ptr);
        }
        goto out;
    }

    app_version = cJSON_GetObjectItemCaseSensitive(json, "version");
    if (!app_version) {
        printf("Error 'version' object not found in JSON\n");
        goto out;
    }

    app_metadata = cJSON_GetObjectItemCaseSensitive(app_version, "app_metadata");
    if (app_metadata && cJSON_IsObject(app_metadata)) {
        name_field = cJSON_GetObjectItemCaseSensitive(app_metadata, "name");
        if (name_field && cJSON_IsString(name_field)) {
            name = name_field->valuestring;
        }

        app_application = cJSON_GetObjectItemCaseSensitive(app_metadata, "application");
        debug_printf("Found 'application' in 'app_metadata'\n");
        if (app_application && cJSON_IsArray(app_application)) {
            debug_printf("Found 'application' in 'app_metadata' is array\n");
            cJSON *application_item = NULL;
            cJSON_ArrayForEach(application_item, app_application) {
                debug_printf("ForEach 'application'\n");
                if (cJSON_IsObject(application_item)) {
                    executable_field = cJSON_GetObjectItemCaseSensitive(application_item, "executable");
                    debug_printf("Found 'executable'\n");
                    if (executable_field && cJSON_IsString(executable_field)) {
                        executable = executable_field->valuestring;
                        debug_printf("Executable field %s\n", executable);
                    }
                }
            }
        }
    }

    files_array = cJSON_GetObjectItemCaseSensitive(app_version, "files");
    if (!files_array) {
        printf("Error: 'files' array not found in JSON\n");
        goto out;
    }

    if (!cJSON_IsArray(files_array)) {
        printf("Error: 'files' is not an array\n");
        goto out;
    }

    result = true;
    cJSON_ArrayForEach(file_item, files_array) {
        if (!cJSON_IsObject(file_item)) {
            printf("Warning: Skipping non-object item in files array\n");
            continue;
        }

        cJSON *file_url  = cJSON_GetObjectItemCaseSensitive(file_item, "url");
        cJSON *full_path = cJSON_GetObjectItemCaseSensitive(file_item, "full_path");

        if (!file_url || !cJSON_IsString(file_url)) {
            printf("Warning: Missing or invalid 'url' field\n");
            result = false;
            continue;
        }

        if (!full_path || !cJSON_IsString(full_path)) {
            printf("Warning: Missing or invalid 'full_path' field\n");
            result = false;
            continue;
        }

        if (!update_application_file(app, full_path->valuestring, file_url->valuestring)) {
            printf("Unable to update file %s\n", full_path->valuestring);
            result = false;
        }
    }

    if (result) {
        application_set_version(app, version);
        application_set_metadata(app, "metadata.json");

        if (name) {
            application_set_name(app, name);
        }

        if (executable) {
            application_set_binary_path(app, executable);
        }
    }
out:
    cJSON_Delete(json);
    free(url);
    free(response_data.memory);
    return result;
}
