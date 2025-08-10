#ifndef APP_CARD_H

#define APP_CARD_H

#include "badgehub_client.h"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>

#define CHAR_WIDTH 8
#define BOX_OFFSET 10
#define CARD_DIST  5

int draw_app_card_detail(SDL_Renderer *renderer, int x, int y, project_detail_t app_info);

int draw_app_card(SDL_Renderer *renderer, int x, int y, project_t app_info, bool show_rect);

int draw_app_list(SDL_Renderer *renderer, int x, int y, project_t *apps, int apps_count, int focused_id);

int draw_app_loading(SDL_Renderer *renderer, int x, int y);
#endif /* end of include guard: APP_CARD_H */
