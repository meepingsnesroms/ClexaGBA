#pragma once

#include "gametypes.h"

extern uint16_t* background;

void init_renderer();

void conv_32bpp_to_16(uint16_t* output, uint32_t* data, uint32_t size);
void invert_color(uint16_t* data, uint32_t size);

void draw_background();
void restore_background(entity& ent);

void draw_texture(uint16_t x, uint16_t y, texture& tex);
void draw_texture_background(uint16_t x, uint16_t y, texture& tex);

void draw_entity(entity& ent);
void draw_entity_background(entity& ent);
