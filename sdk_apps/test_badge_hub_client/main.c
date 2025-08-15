#include "app_card.h"
// #include "SDL3/SDL_thread.h"
// #include "badgehub_client.h"
#include "badgehub_client_new.h"
#include "badgevms/process.h"
#include "down_man.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_scancode.h"
#include "sys/unistd.h"
#include "test_drawing_helper.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <unistd.h>
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


#define WINDOW_WIDTH   720
#define WINDOW_HEIGHT  720
#define DISPLAY_ID     0
#define ADJUST_DISPLAY 0



/*! \enum UIStates
 *
 *  States the UI could be in.
 */
typedef enum {
    APP_LIST,
    APP_DETAIL,
    APP_INSTALL,
} UIStates;

typedef struct {
    project_detail_t *app;
    char             *current_file_name;
    int               got_files;
    bool              error;
} ThreadData;

/*! \struct AppState
 *  \brief State of the Badgehub Client needed across threads
 */
typedef struct {
    SDL_Window   *sdl_window;
    SDL_Renderer *sdl_renderer;
    int           window_width;
    int           window_height;
    int           text_x;
    int           text_y;
    project_t    *apps;
    int           app_count;
    int           selected;
    bool          install;
    pid_t         sdl_install_thread;
    ThreadData   *thread_data;
    UIStates      state;
} AppState;



SDL_AppResult handle_key_event_(void **appstate, SDL_Scancode key_code) {
    AppState *as = (AppState *)appstate;
    switch (key_code) {
        /* Quit. */
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_Q: return SDL_APP_SUCCESS;
        /* Decide new direction of the snake. */
        case SDL_SCANCODE_RIGHT: as->text_x += 5; break;
        case SDL_SCANCODE_UP: as->selected = as->selected - 1 < 0 ? 0 : as->selected - 1; break;
        case SDL_SCANCODE_LEFT: as->text_x -= 5; break;
        case SDL_SCANCODE_DOWN:
            as->selected = as->selected + 1 >= as->app_count ? as->app_count - 1 : as->selected + 1;
            break;
        case SDL_SCANCODE_SPACE:
        case SDL_SCANCODE_KP_ENTER: as->install = true;
        default: break;
    }
    return SDL_APP_CONTINUE;
}
/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    AppState     *as              = NULL;
    project_t    *projects        = NULL;
    char         *query           = "";
    int           num_of_projects = -1;
    int           d_number_of     = -1;
    SDL_AppResult result          = SDL_APP_FAILURE;

    SDL_SetAppMetadata("Test Badgehub Client", "1.0", "test-debug-renderer");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        goto out;
    }
    // allocate appstate and reassign
    as = (AppState *)SDL_calloc(1, sizeof(AppState));
    if (!as) {
        result = SDL_APP_FAILURE;
        goto out;
    }
    as->window_height = WINDOW_HEIGHT / 2;
    as->window_width  = WINDOW_WIDTH / 2;
    as->text_x        = 100;
    as->text_y        = 100;
    *appstate         = as;

    if (ADJUST_DISPLAY) {
        SDL_DisplayID *d_ids = SDL_GetDisplays(&d_number_of);
        if (d_ids || d_number_of) {

            SDL_DisplayMode const *d_mode = SDL_GetCurrentDisplayMode(d_ids[DISPLAY_ID]);

            if (d_mode) {
                // increase font size
                as->window_width  = d_mode->w / 2;
                as->window_height = d_mode->h / 2;
            } else {
                printf(
                    "Couldn't get display informations for display %d. Number of Displays: %d\n",
                    DISPLAY_ID,
                    d_number_of
                );
            }
        } else {
            printf("Couldn't get SDL displays.\n");
        }
        SDL_free(d_ids);
    }

    printf("Getting applications\n");
    projects = get_projects(query, 50, 0, &num_of_projects);
    printf("%x\n", projects);
    printf("Number of Project: %d\n", num_of_projects);
    if (projects) {
        printf("1. Project name: %s\n", projects[0].name);
        as->apps      = projects;
        as->app_count = num_of_projects;
    } else {
        printf("ERROR getting applications!\n");
    }

    // draw window
    if (!SDL_CreateWindowAndRenderer(
            "TestBadgehubClient",
            as->window_width,
            as->window_height,
            // 0,
            SDL_WINDOW_FULLSCREEN,
            &as->sdl_window,
            &as->sdl_renderer
        )) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        result = SDL_APP_FAILURE;
        goto out;
    }

    SDL_SetRenderLogicalPresentation(
        as->sdl_renderer,
        as->window_width,
        as->window_height,
        SDL_LOGICAL_PRESENTATION_INTEGER_SCALE
    );
    result = SDL_APP_CONTINUE; /* carry on with the program! */

out:
    return result;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT: return SDL_APP_SUCCESS;
        case SDL_EVENT_KEY_DOWN: return handle_key_event_(appstate, event->key.scancode);
        default: return SDL_APP_CONTINUE;
    }
}

SDL_AppResult draw_main_screen(AppState *as) {
    SDL_SetRenderDrawColor(as->sdl_renderer, 255, 255, 194, SDL_ALPHA_OPAQUE);
    SDL_FRect r = {0, 0, as->window_width, as->window_height};
    SDL_RenderFillRect(as->sdl_renderer, &r);


    draw_app_list(as->sdl_renderer, 10, 10, as->apps, as->app_count, as->selected);

    return SDL_APP_CONTINUE;
}

void install_app(void *data) {
    ThreadData *t_data = NULL;
    if (data) {
        t_data = (ThreadData *)data;
    } else {
        printf("No data given to thread, exiting...\n");
        t_data->error = true;
    }
    printf("Start downloading..\n");
    sleep(5);
    if (!install_application_file(t_data->app, &(t_data->got_files), &(t_data->current_file_name))) {
        printf("Unable to install files!!\n");
        t_data->error = true;
    }
    printf("Finished downloading..\n");
    t_data->error = false;
}


SDL_AppResult draw_app_install(AppState *as) {
    SDL_AppResult result = SDL_APP_CONTINUE;
    char         *text   = NULL;
    pid_t         pid    = wait(false, 100);
    if (pid == as->sdl_install_thread) {
        int state = as->thread_data->error;
        if (state) {
            draw_app_loading(as->sdl_renderer, as->window_width / 4, as->window_width / 4, "ERROR Downloading!");
            sleep(1);
            result = SDL_APP_FAILURE;
            printf("Error in Thread!\n");
        } else {
            draw_app_loading(as->sdl_renderer, as->window_width / 4, as->window_width / 4, "Finished Downloading!");
            sleep(1);
            printf("Finished Installing\n");
        }
        as->install = false;
    }
    if (as->install) {
        asprintf(
            &text,
            "Downloading '%s'... (%d/%d)",
            as->thread_data->current_file_name,
            as->thread_data->got_files,
            as->thread_data->app->file_count
        );
        draw_app_loading(as->sdl_renderer, as->window_width / 4, as->window_width / 4, text);
    }
    return result;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate) {

    AppState *as = (AppState *)appstate;

    // #ffffcc
    draw_main_screen(as);

    // draw_text(as->sdl_renderer, "This is a renderer Text, how amazing is that!!", as->text_x, as->text_y, 0x0);


    if (as->install) {
        if (!as->sdl_install_thread) {
            project_t         selected = as->apps[as->selected];
            project_detail_t *detail   = get_project_details(selected.slug, selected.revision);
            if (!detail) {
                printf("Error getting project detail for '%s'\n", selected.slug);
                as->install = false;
            }
            if (!as->thread_data) {
                as->thread_data                    = malloc(sizeof(ThreadData));
                as->thread_data->current_file_name = "";
                as->thread_data->app               = detail;
                as->thread_data->got_files         = -1;
            }

            as->sdl_install_thread = thread_create(&install_app, as->thread_data, 8192);
            if (!as->sdl_install_thread) {
                printf("Error Starting Install Thread \n");
                as->install = false;
            }
        }

        if (draw_app_install(as) == SDL_APP_FAILURE)
            printf("Failed draw app installing!\n");
    }

    SDL_RenderPresent(as->sdl_renderer); /* put it all on the screen! */

    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {

    // cleanup appstate
    if (appstate != NULL) {
        AppState *as = (AppState *)appstate;
        SDL_DestroyRenderer(as->sdl_renderer);
        SDL_DestroyWindow(as->sdl_window);
        SDL_free(as);
        wait(true, 1000);
    }
    /* SDL will clean up the window/renderer for us. */
}
