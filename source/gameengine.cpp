#include <gba_video.h>
#include <gba_input.h>
#include <stdlib.h>

//#include <vector>
#include "../data/clarke.cdata"
#include "../data/clexalogo.cdata"
#include "../data/crosshair.cdata"
#include "../data/polis.cdata"

#include "ugui/ugui.h"//for color defines

//use framebuffer for now
static uint16_t *const vram = ((uint16_t*)0x06000000);
static uint16_t keys;

#define ENTITYS 20
#define PLAYER characters[0]

uint16_t* bitmap_conv_ram;//[SCREEN_WIDTH * SCREEN_HEIGHT];
uint16_t* background;//[SCREEN_WIDTH * SCREEN_HEIGHT];
uint16_t  playermap[16 * 16];

unsigned int cust_rand(){
   static unsigned int z1 = 12345, z2 = 12345, z3 = 12345, z4 = 12345;
   unsigned int b;
   b  = ((z1 << 6) ^ z1) >> 13;
   z1 = ((z1 & 4294967294U) << 18) ^ b;
   b  = ((z2 << 2) ^ z2) >> 27;
   z2 = ((z2 & 4294967288U) << 2) ^ b;
   b  = ((z3 << 13) ^ z3) >> 21;
   z3 = ((z3 & 4294967280U) << 7) ^ b;
   b  = ((z4 << 3) ^ z4) >> 12;
   z4 = ((z4 & 4294967168U) << 13) ^ b;
   return (z1 ^ z2 ^ z3 ^ z4);
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
   uint16_t* bitmap;
}entity;

void conv_32bpp_to_16(uint16_t* output, uint32_t* data, uint32_t size){
   for(uint32_t cnt = 0; cnt < size; cnt++){
      uint8_t visible = (data[cnt] >> 24) & 0xFF;
      if(visible != 0){
         uint8_t r = (data[cnt] >> 16) & 0xFF;
         uint8_t g = (data[cnt] >> 8) & 0xFF;
         uint8_t b = data[cnt] & 0xFF;
         output[cnt] = RGB8(b,g,r) | 0x8000/*visible bit*/;
      }
      else{
         //if invisible no need to calculate what shade of invisible :)
         output[cnt] = 0x0000;
      }
   }
}

void draw_square(uint16_t x, uint16_t y, uint16_t color){
   for(uint16_t yinc = 0; yinc < 16; yinc++){
      for(uint16_t xinc = 0; xinc < 16; xinc++){
         vram[x + xinc + ((y + yinc) * SCREEN_WIDTH)] = color;
      }
   }
}

void restore_square(uint16_t x, uint16_t y){
   for(uint16_t yinc = 0; yinc < 16; yinc++){
      for(uint16_t xinc = 0; xinc < 16; xinc++){
         vram[x + xinc + ((y + yinc) * SCREEN_WIDTH)] = background[x + xinc + ((y + yinc) * SCREEN_WIDTH)];
      }
   }
}

void draw_entity(entity& ent){
   for(uint16_t yinc = 0; yinc < ent.h; yinc++){
      for(uint16_t xinc = 0; xinc < ent.w; xinc++){
         //check "is visible" bit
         uint16_t color = ent.bitmap[xinc + (yinc * ent.w)];
         if(color & 0x8000){
            vram[ent.x + xinc + ((ent.y + yinc) * SCREEN_WIDTH)] = color & 0x7FFF;//ent.bitmap[xinc + (yinc * ent.w)];
         }
      }
   }
}

void draw_entity_background(entity& ent){
   for(uint16_t yinc = 0; yinc < ent.h; yinc++){
      for(uint16_t xinc = 0; xinc < ent.w; xinc++){
         //check "is visible" bit
         uint16_t color = ent.bitmap[xinc + (yinc * ent.w)];
         if(color & 0x8000){
            background[ent.x + xinc + ((ent.y + yinc) * SCREEN_WIDTH)] = color & 0x7FFF;//ent.bitmap[xinc + (yinc * ent.w)];
         }
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

void draw_logo(){
   entity polis;
   conv_32bpp_to_16(bitmap_conv_ram, (uint32_t*)polis_data[0], POLIS_FRAME_WIDTH * POLIS_FRAME_HEIGHT);
   polis.x = 0;
   polis.y = 20;//gba is 160 tall, image is 135
   polis.w = POLIS_FRAME_WIDTH;
   polis.h = POLIS_FRAME_HEIGHT;
   polis.bitmap = bitmap_conv_ram;
   polis.active = true;
   draw_entity_background(polis);
   
   /*
   entity logo;
   conv_32bpp_to_16(bitmap_conv_ram, (uint32_t*)clexa_logo_data[0], CLEXA_LOGO_FRAME_WIDTH * CLEXA_LOGO_FRAME_HEIGHT);
   //need to fix starting location
   logo.x = 0;
   logo.y = 0;
   logo.w = CLEXA_LOGO_FRAME_WIDTH;
   logo.h = CLEXA_LOGO_FRAME_HEIGHT;
   logo.bitmap = bitmap_conv_ram;
   logo.active = true;
   draw_entity_background(logo);
   */
}

void init_game(){
   bitmap_conv_ram = (uint16_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));
   background = (uint16_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));
   
   if(background == NULL || bitmap_conv_ram == NULL){
      while(1);//abort
   }
   
   //make background random
   for(uint32_t cnt = 0; cnt < SCREEN_WIDTH * SCREEN_HEIGHT; cnt++){
      background[cnt] = cust_rand() % 0x7FFF;
   }
   
   //polis tower
   draw_logo();
   
   
   //conv_32bpp_to_16(playermap, (uint32_t*)clarke_data[0], 16 * 16);
   conv_32bpp_to_16(playermap, (uint32_t*)crosshair_data[0], 16 * 16);
   
   PLAYER.active = true;
   PLAYER.x = 0;
   PLAYER.y = 0;
   PLAYER.w = 16;
   PLAYER.h = 16;
   PLAYER.bitmap = playermap;
}

void switch_to_game(){
   for(uint32_t cnt = 0; cnt < SCREEN_WIDTH * SCREEN_HEIGHT; cnt++){
      vram[cnt] = background[cnt];
   }
}

void run_frame_game(){
   keys = ~(REG_KEYINPUT);
   
   //draw_square(PLAYER.x, PLAYER.y, C_BLACK);
   restore_square(PLAYER.x, PLAYER.y);
   
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
   
   if(keys & KEY_B){
      draw_logo();
   }
   
   draw_entity(PLAYER);
}
