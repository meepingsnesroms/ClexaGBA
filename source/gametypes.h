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
   int32_t w;//width
   int32_t h;//height
   uint16_t* bitmap;
}texture;

typedef struct{
   int32_t  x;//current x coord
   int32_t  y;//current y coord
   int32_t  w;//width
   int32_t  h;//height
   struct{
      int32_t x;//x start of corrupted background
      int32_t y;//y start of corrupted background
      bool is_dirty;
   }dirty;
   int32_t accel_x;//how far to move on x axis per frame -127<->126
   int32_t accel_y;//how far to move on y axis per frame -127<->126
   int32_t angle;//degrees 0<->359
   int32_t gravity;//negitive gravity sends you upward
   bool active;//if this is entity is currently in use
   bool kill_on_exit;//if this is set remove the entity from the list on level exit
   bool bullet;
   bool is_solid;
   int32_t health;
   int32_t index;
   int32_t sprite_x_offset;
   int32_t sprite_y_offset;
   texture sprite;
   void (*frame_iterate)(void* me);
}entity;

typedef struct{
   int32_t w;
   int32_t h;
   int32_t frames;
   uint16_t** bitmaps;
}animation;

typedef struct{
   int32_t level_id;//this levels level id
   
   int32_t north;//level number above this level
   int32_t south;//level number below this level
   int32_t east;//level number to the right of this level
   int32_t west;//level number to the left of this level
   
   entity*  sprites;
   int32_t num_sprites;
   
   uint16_t* background;//background bitmap
   uint16_t* foreground;//if a pixel is not transparent here it is solid in the collision map
}level;




static inline uint16_t get_texture_pixel(int32_t x, int32_t y, texture& tex){
   int32_t cropped_x = x % tex.w;
   int32_t cropped_y = y % tex.h;
   return tex.bitmap[cropped_y * tex.w + cropped_x];
}
