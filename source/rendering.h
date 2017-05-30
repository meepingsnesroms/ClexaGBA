#pragma once

#include "gametypes.h"

extern uint16_t* background;

void init_renderer();

void conv_32bpp_to_16(uint16_t* output, uint32_t* data, uint32_t size);
void invert_color(uint16_t* data, uint32_t size);
uint16_t blend_color(uint16_t arg1, uint16_t arg2);

void draw_background();
void fill_background(uint16_t color);
void restore_background(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void restore_background(entity& ent);

void draw_texture(int16_t x, int16_t y, texture& tex);
void draw_texture_background(uint16_t x, uint16_t y, texture& tex);

void draw_entity(entity& ent);
void draw_entity_background(entity& ent);
