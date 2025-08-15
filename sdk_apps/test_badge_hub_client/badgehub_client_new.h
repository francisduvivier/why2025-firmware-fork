#ifndef BADGEHUB_CLIENT_NEW_H

#define BADGEHUB_CLIENT_NEW_H

// Represents a project summary from the main project list.
typedef struct {
    char *name;
    char *slug;
    char *description;
    char *project_url;
    char *icon_url;
    int   revision;
} project_t;

// Represents a single file within a project.
typedef struct {
    char *full_path;
    char *sha256;
    char *url;
} project_file_t;

// Represents the detailed information for a single project.
typedef struct {
    char           *name;
    char           *description;
    char           *published_at;
    char           *author;
    char           *version;
    char           *slug;
    int             revision;
    project_file_t *files;
    int             file_count;
} project_detail_t;

// Queries the badgehub API for available projects with a limit on the query at once and an offset.
// returns the number of queried project or -1 on error
project_t *get_projects(char const *search_query, int limit, int offset, int *out_num_of_projects);

// Queries the badgehub API for a project and returns all details of the project from a specified revision, returns
// project details or NULL on error
project_detail_t *get_project_details(char const *slug, int revision);

// Queries the badgehub API for all project files from the project slug, returns number of files or -1 on error
int get_projects_files(char const *slug, int revision, project_file_t *out_project_files);

// freeing memory
void free_project_details(project_detail_t *details);
void free_projects(project_t *details, int count);
void free_project_file(project_detail_t *details);

#endif /* end of include guard: BADGEHUB_CLIENT_NEW_H */
