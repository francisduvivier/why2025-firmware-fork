#include "app_card.h"
#include "badgehub_client.h"
#include "down_man.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_scancode.h"
#include "sys/unistd.h"
#include "test_drawing_helper.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_video.h>
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


#define WINDOW_WIDTH  720
#define WINDOW_HEIGHT 720
#define DISPLAY_ID    0


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
    SDL_SetAppMetadata("Test Badgehub Client", "1.0", "test-debug-renderer");



    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    // allocate appstate and reassign
    AppState *as = (AppState *)SDL_calloc(1, sizeof(AppState));
    if (!as) {
        return SDL_APP_FAILURE;
    }
    as->window_height = WINDOW_HEIGHT;
    as->window_width  = WINDOW_WIDTH;
    as->text_x        = 100;
    as->text_y        = 100;
    *appstate         = as;

    //
    int           *d_number_of = NULL;
    SDL_DisplayID *d_ids       = SDL_GetDisplays(d_number_of);
    if (d_ids || d_number_of) {

        SDL_DisplayMode const *d_mode = SDL_GetCurrentDisplayMode(d_ids[DISPLAY_ID]);

        if (d_mode) {
            as->window_width  = d_mode->w / 2;
            as->window_height = d_mode->h / 2;
        } else {
            // TODO
        }
    } else {
        // TODO
    }

    char *query = "";

    printf("Getting applications\n");
    project_t *p = get_applications(&as->app_count, query, 50, 0);
    if (p) {
        printf("1. Project name: %s\n", p[0].name);
        as->apps = p;
    } else {
        printf("ERROR getting applications!\n");
    }

    // draw window
    if (!SDL_CreateWindowAndRenderer(
            "TestBadgehubClient",
            as->window_width,
            as->window_height,
            SDL_WINDOW_FULLSCREEN,
            &as->sdl_window,
            &as->sdl_renderer
        )) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderLogicalPresentation(
        as->sdl_renderer,
        as->window_width,
        as->window_height,
        SDL_LOGICAL_PRESENTATION_INTEGER_SCALE
    );

    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT: return SDL_APP_SUCCESS;
        case SDL_EVENT_KEY_DOWN: return handle_key_event_(appstate, event->key.scancode);
        default: return SDL_APP_CONTINUE;
    }
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate) {

    // printf("AppIterate...\n");
    AppState *as = (AppState *)appstate;

    // #ffffcc
    SDL_SetRenderDrawColor(as->sdl_renderer, 255, 255, 194, SDL_ALPHA_OPAQUE);
    SDL_FRect r = {0, 0, as->window_width, as->window_height};
    SDL_RenderFillRect(as->sdl_renderer, &r);


    draw_app_list(as->sdl_renderer, 10, 10, as->apps, as->app_count, as->selected);

    // draw_text(as->sdl_renderer, "This is a renderer Text, how amazing is that!!", as->text_x, as->text_y, 0x0);

    SDL_RenderPresent(as->sdl_renderer); /* put it all on the screen! */

    if (as->install) {
        printf("Start downloading..\n");
        project_t         selected         = as->apps[as->selected];
        project_detail_t *detail           = get_project_details(selected.slug, selected.revision);
        int               downloaded_files = 0;
        draw_app_loading(as->sdl_renderer, WINDOW_WIDTH / 4, WINDOW_HEIGHT / 4);
        if (!install_application_file(detail, &downloaded_files)) {
            printf("Unable to install files!!\n");
        } else {
            while (downloaded_files < detail->file_count) {
                printf("Got files: %d\n", downloaded_files);
                sleep(1);
            }
        }
        as->install = false;
        free_project_details(detail);
    }

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
    }
    /* SDL will clean up the window/renderer for us. */
}



//
// int main(int argc, char **argv)
// {
//     (void)argc;
//     (void)argv;
//
//     lv_init();
//     hal_init(720, 720);
//
//     // Create the main application UI using the new home screen
//     create_app_home_view();
//
//     while (1)
//     {
//         lv_timer_handler();
// #ifdef _MSC_VER
//         Sleep(5);
// #else
//         usleep(5 * 1000);
// #endif
//     }
//
//     return 0;
// }
//
// static lv_display_t *hal_init(int32_t w, int32_t h)
// {
//     lv_group_set_default(lv_group_create());
//     lv_display_t *disp = lv_sdl_window_create(w, h);
//     lv_indev_t *mouse = lv_sdl_mouse_create();
//     lv_indev_set_group(mouse, lv_group_get_default());
//     lv_indev_set_display(mouse, disp);
//     lv_display_set_default(disp);
//     lv_indev_t *mousewheel = lv_sdl_mousewheel_create();
//     lv_indev_set_display(mousewheel, disp);
//     lv_indev_set_group(mousewheel, lv_group_get_default());
//     lv_indev_t *kb = lv_sdl_keyboard_create();
//     lv_indev_set_display(kb, disp);
//     lv_indev_set_group(kb, lv_group_get_default());
//     return disp;
// }
