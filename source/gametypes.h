#pragma once

#include <stdint.h>

#include "trig.h"

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
   //variables
   
   union{
      //current x coord, bullets need fractional numbers due to use of sin and cos
      int32_t  x;
      fixedpt  fxd_x;
   };
   union{
      //current y coord, bullets need fractional numbers due to use of sin and cos
      int32_t  y;
      fixedpt  fxd_y;
   };
   
   int32_t  w;//width
   int32_t  h;//height
   
   union{
      //how far to move on x axis per frame, bullets need fractional numbers due to use of sin and cos
      int32_t accel_x;
      fixedpt fxd_accel_x;
   };
   union{
      //how far to move on y axis per frame, bullets need fractional numbers due to use of sin and cos
      int32_t accel_y;
      fixedpt fxd_accel_y;
   };
   
   int32_t angle;//degrees 0<->359
   int32_t gravity;//negitive gravity sends you upward
   
   struct{
      int32_t x;//x start of corrupted background
      int32_t y;//y start of corrupted background
      bool is_dirty;
   }dirty;
   
   bool active;//if this is entity is currently in use
   bool kill_on_exit;//if this is set remove the entity from the list on level exit
   bool bullet;
   bool is_enemy;
   bool is_solid;
   int32_t health;
   int32_t index;
   int32_t sprite_x_offset;
   int32_t sprite_y_offset;
   texture sprite;
   void*   data;//used if external data(such as a menu item) needs to be linked to this entity
   
   //callbacks
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
