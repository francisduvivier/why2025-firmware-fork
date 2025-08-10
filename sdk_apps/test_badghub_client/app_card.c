#include "app_card.h"

#include "test_drawing_helper.h"

#include <stdio.h>

#include <SDL3/SDL_rect.h>
#include <string.h>


int draw_app_card(SDL_Renderer *renderer, int x, int y, project_t app_info, bool show_rect) {
    if (show_rect) {
        int             len = strlen(app_info.name);
        SDL_FRect const r   = {.x = x, .y = y, .h = CHAR_WIDTH + BOX_OFFSET, .w = len * CHAR_WIDTH + BOX_OFFSET};

        SDL_SetRenderDrawColor(renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        draw_text(renderer, app_info.name, x + BOX_OFFSET / 2, y + BOX_OFFSET / 2, 0xFFFFFF);
        // printf("X: %d, Y: %d\n", x, y);
    } else {
        draw_text(renderer, app_info.name, x + BOX_OFFSET / 2, y + BOX_OFFSET / 2, 0x0);
    }
}

int draw_app_list(SDL_Renderer *renderer, int x, int y, project_t *apps, int apps_count, int focused_id) {

    int scale  = (CHAR_WIDTH + BOX_OFFSET + CARD_DIST);
    int offset = 0;


    if (focused_id * scale >= 300) {
        offset = focused_id - 14;
    }
    for (int i = offset; i < apps_count; i++) {
        int shift = i - offset;
        draw_app_card(renderer, x, y + shift * scale, apps[i], focused_id == i);
    }
}

int draw_app_loading(SDL_Renderer *renderer, int x, int y) {

    int             len = strlen("Loading...");
    SDL_FRect const r   = {.x = x, .y = y, .h = CHAR_WIDTH + BOX_OFFSET, .w = len * CHAR_WIDTH + BOX_OFFSET};
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &r);
    draw_text(renderer, "Loading...", x + BOX_OFFSET / 2, y + BOX_OFFSET / 2, 0xFFFFFF);
}
