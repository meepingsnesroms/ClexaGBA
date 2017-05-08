#include <gba_video.h>
#include <gba_input.h>

//#include <vector>
#include "../data/clarke.cdata"

#include "ugui/ugui.h"//for color defines

//use framebuffer for now
static uint16_t *const vram = ((uint16_t*)0x06000000);
static uint16_t keys;

#define ENTITYS 20
#define PLAYER characters[0]

uint16_t playermap[16 * 16];

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
   uint16_t* bitmap;
}entity;

void conv_32bpp_to_16(uint16_t* output, uint32_t* data, uint32_t size){
   for(uint32_t cnt = 0; cnt < size; cnt++){
      uint8_t r = (data[cnt] >> 16) & 0xFF;
      uint8_t g = (data[cnt] >> 8) & 0xFF;
      uint8_t b = data[cnt] & 0xFF;
      output[cnt] = RGB8(b,g,r);
   }
}

void draw_square(uint16_t x, uint16_t y, uint16_t color){
   for(uint16_t yinc = 0; yinc < 16; yinc++){
      for(uint16_t xinc = 0; xinc < 16; xinc++){
         vram[x + xinc + ((y + yinc) * SCREEN_WIDTH)] = color;
      }
   }
}

void draw_entity(entity& ent){
   for(uint16_t yinc = 0; yinc < ent.h; yinc++){
      for(uint16_t xinc = 0; xinc < ent.w; xinc++){
         vram[ent.x + xinc + ((ent.y + yinc) * SCREEN_WIDTH)] = ent.bitmap[xinc + (yinc * ent.w)];
      }
   }
}

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

/*
bool collision_test(entity& chr1, entity& chr2){
   
}
*/

entity   characters[ENTITYS];
entity*  active_characters[ENTITYS];
uint8_t  num_active_characters;

void cull_characters(){
   
}

void init_game(){
   //SetMode(MODE_3 | BG2_ON /* 240*160 16bit color */);
   
   conv_32bpp_to_16(playermap, (uint32_t*)clarke_data[0], 16 * 16);
   
   PLAYER.active = true;
   PLAYER.x = 0;
   PLAYER.y = 0;
   PLAYER.w = 16;
   PLAYER.h = 16;
   PLAYER.bitmap = playermap;
}

void switch_to_game(){
   
}

void run_frame_game(){
   keys = ~(REG_KEYINPUT);
   
   draw_square(PLAYER.x, PLAYER.y, C_BLACK);
   
   if(keys & KEY_LEFT){
      if(PLAYER.x > 0){
         PLAYER.x--;
      }
   }
   if(keys & KEY_RIGHT){
      if(PLAYER.x + PLAYER.w < SCREEN_WIDTH){
         PLAYER.x++;
      }
   }
   if(keys & KEY_UP){
      if(PLAYER.y > 0){
         PLAYER.y--;
      }
   }
   if(keys & KEY_DOWN){
      if(PLAYER.y + PLAYER.h < SCREEN_HEIGHT){
         PLAYER.y++;
      }
   }
   
   draw_entity(PLAYER);
}
