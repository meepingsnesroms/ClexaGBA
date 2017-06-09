#pragma once

#include <stdint.h>

enum{
   DIR_NONE,
   DIR_UP,
   DIR_DOWN,
   DIR_LEFT,
   DIR_RIGHT
};

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
   bool kill_on_exit;//if this is set remove the entity from the list on level exit
   bool bullet;
   bool is_solid;
   uint16_t health;
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
   uint16_t level_id;//this levels level id
   
   uint16_t north;//level number above this level
   uint16_t south;//level number below this level
   uint16_t east;//level number to the right of this level
   uint16_t west;//level number to the left of this level
   
   entity*  sprites;
   uint16_t num_sprites;
   
   uint16_t* background;//background bitmap
   uint16_t* foreground;//if a pixel is not transparent here it is solid in the collision map
}level;




static inline uint16_t get_texture_pixel(uint16_t x, uint16_t y, texture& tex){
   uint16_t cropped_x = x % tex.w;
   uint16_t cropped_y = y % tex.h;
   return tex.bitmap[cropped_x + cropped_y * tex.w];
}
