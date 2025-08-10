#pragma once

// #include "badgevms/framebuffer.h"
// #include "thirdparty/microui.h"

#include <stdint.h>
#include <SDL3/SDL_render.h>
static uint8_t const font_8x8[96][8];
void                 draw_text(SDL_Renderer *fb, char const *text, int x, int y, uint32_t color);
void                 draw_char(SDL_Renderer *fb, char c, int x, int y, uint32_t color);
// void                 draw_rect(framebuffer_t *fb, mu_Rect rect, mu_Color color);
void                 draw_special_symbol(SDL_Renderer *fb, int symbol_index, int x, int y, uint32_t color);
int                  get_special_symbol_index(char const *symbol);
// int                  mu_text_height(mu_Font font);
// int                  mu_text_width(mu_Font font, char const *text, int len);
