#pragma once

#include <stdint.h>

typedef struct{
   uint16_t w;//width
   uint16_t h;//height
   uint16_t* bitmap;
}texture;

typedef struct{
   uint16_t x;//current x coord
   uint16_t y;//current y coord
   uint16_t w;//width
   uint16_t h;//height
   struct{
      uint16_t x;//x start of corrupted background
      uint16_t y;//y start of corrupted background
      bool is_dirty;
   }dirty;
   int8_t accel_x;//how far to move on x axis per frame -127<->126
   int8_t accel_y;//how far to move on y axis per frame -127<->126
   uint16_t angle;//degrees 0<->359
   int16_t gravity;//negitive gravity sends you upward
   bool active;//if this is entity is currently in use
   bool bullet;
   bool is_hit;
   bool is_solid;
   int8_t index;
   int8_t sprite_x_offset;
   int8_t sprite_y_offset;
   texture sprite;
   void (*frame_iterate)(void* me);
}entity;

typedef struct{
   uint16_t w;
   uint16_t h;
   uint8_t frames;
   uint16_t** bitmaps;
}animation;

typedef struct{
   uint16_t north;
   uint16_t south;
   uint16_t east;
   uint16_t west;
   
   uint16_t* bitmap;
   uint8_t*  collison_map;
}level_map;




static inline uint16_t get_texture_pixel(uint16_t x, uint16_t y, texture& tex){
   uint16_t cropped_x = x % tex.w;
   uint16_t cropped_y = y % tex.h;
   return tex.bitmap[cropped_x + cropped_y * tex.w];
}
