#pragma once

#include <gba_video.h>

#include "gametypes.h"

extern uint16_t* vram;
extern uint16_t* background;

void init_renderer();

void invert_color(uint16_t* data, uint32_t size);
uint16_t blend_color(uint16_t arg1, uint16_t arg2);

void draw_background();
void fill_background(uint16_t color);
void restore_background(int32_t x, int32_t y, int32_t w, int32_t h);
void restore_background(entity& ent);

void draw_texture(int32_t x, int32_t y, texture& tex);
void draw_texture_from_midpoint(int32_t x, int32_t y, texture& tex);
void draw_texture_background(int32_t x, int32_t y, texture& tex);

void draw_entity(entity& ent);
void draw_entity_background(entity& ent);

inline void plot_vram_pixel(int32_t x, int32_t y, uint16_t color){vram[x + (y * SCREEN_WIDTH)] = color;}
inline void restore_background_pixel(int32_t x, int32_t y){vram[x + (y * SCREEN_WIDTH)] = background[x + (y * SCREEN_WIDTH)];}
