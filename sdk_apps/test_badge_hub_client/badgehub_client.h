#ifndef BADGEHUB_CLIENT_H
#define BADGEHUB_CLIENT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "badgehub_client_new.h"



project_t *get_applications(int *project_count, const char* search_query, int limit, int offset);
void free_applications(project_t *projects, int count);
project_detail_t *get_project_details(const char *slug, int revision);
void free_project_details(project_detail_t *details);
bool download_project_file(const project_file_t* file_info, const char* project_slug);

/**
 * @brief Downloads an icon from a URL into a memory buffer.
 *
 * @param icon_url The direct URL of the icon to download.
 * @param data_size A pointer that will be populated with the size of the downloaded data.
 * @return A dynamically allocated buffer with the raw PNG data, or NULL on failure. The caller must free this buffer.
 */
uint8_t* download_icon_to_memory(const char* icon_url, int* data_size);

// bool do_http(char const *url, http_data_t *response_data, http_file_t *http_file);

#endif // BADGEHUB_CLIENT_H
//
