#include <gba_video.h>
#include <gba_input.h>

//#include <vector>

#include "ugui/ugui.h"//for color defines

#define ENTITYS 20
#define PLAYER characters[0]

//use framebuffer for now
static uint16_t *const vram = ((uint16_t*)0x06000000);
static uint16_t keys;


void draw_square(uint16_t x, uint16_t y, uint16_t color){
   for(uint16_t yinc = 0; yinc < 16; yinc++){
      for(uint16_t xinc = 0; xinc < 16; xinc++){
         vram[x + xinc + ((y + yinc) * SCREEN_WIDTH)] = color;
      }
   }
}

typedef struct{
   uint16_t x;//current x coord
   uint16_t y;//current y coord
   uint16_t w;//width
   uint16_t h;//height
   int8_t accel_x;//how far to move on x axis per frame -127<->126
   int8_t accel_y;//how far to move on y axis per frame -127<->126
   uint16_t angle;//degrees 0<->359
   bool active;//if this is entity is currently in use
   bool bullet;
   void* bitmap;
}entity;

/*
entity default_obj_state{
   .x = 0;
   .y = 0;
   .w = 0;
   .h = 0;
   .accel_x = 0;
   .accel_y = 0;
   .angle = 0;
   .active = false;
   .bullet = false;
   .bitmap = NULL;
};
*/

bool collision_test(entity& chr1, entity& chr2){
   
}

entity   characters[ENTITYS];
entity*  active_characters[ENTITYS];
uint8_t  num_active_characters;

void cull_characters(){
   
}

void init_game(){
   //characters.push_back();
   
   PLAYER.active = true;
   
   //SetMode(MODE_3 | BG2_ON /* 240*160 16bit color */);
}

void switch_to_game(){
   
}

void run_frame_game(){
   keys = ~(REG_KEYINPUT);
   
   draw_square(PLAYER.x, PLAYER.y, C_BLACK);
   
   if(keys & KEY_LEFT){
      PLAYER.x--;
   }
   if(keys & KEY_RIGHT){
      PLAYER.x++;
   }
   if(keys & KEY_UP){
      PLAYER.y--;
   }
   if(keys & KEY_DOWN){
      PLAYER.y++;
   }
   
   draw_square(PLAYER.x, PLAYER.y, C_WHITE);
   
}
