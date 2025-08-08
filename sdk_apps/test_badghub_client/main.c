#include "SDL3/SDL_rect.h"
#include "badgehub_client.h"
#include <stdio.h>
#include <stdlib.h>
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

/* We will use this renderer to draw into this window every frame. */
static SDL_Window   *window   = NULL;
static SDL_Renderer *renderer = NULL;

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("Example Renderer Debug Texture", "1.0", "com.example.renderer-debug-text");

  int project_count = 1;

  char *query = "";

  printf("Getting applications\n");
  project_t *p = get_applications(&project_count, query , 5, 0);
  // printf("Project name: %p\n", p[0].name);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer(
            "examples/renderer/debug-text",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            0,
            &window,
            &renderer
        )) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS; /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate) {
    int const charsize = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

    /* as you can see from this, rendering draws over whatever was drawn before it. */
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);  /* black, full alpha */
    // SDL_RenderClear(renderer);  /* start with a blank canvas. */


    SDL_SetRenderDrawColor(renderer, 51, 102, 255, SDL_ALPHA_OPAQUE); /* light blue, full alpha */
    SDL_FRect r = {100, 100, 20, 20};
    SDL_RenderFillRect(renderer, &r);

    SDL_RenderPresent(renderer); /* put it all on the screen! */

    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
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
