#include <gba_video.h>
#include <gba_input.h>
#include <gba_systemcalls.h>
#include <stdlib.h>
#include <string.h>

#include "../data/clarke.cdata"
#include "../data/clexalogo.cdata"
#include "../data/crosshair.cdata"
#include "../data/polis.cdata"

#include "menu.h"
#include "ugui/ugui.h"//for color defines

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
   bool active;//if this is entity is currently in use
   bool bullet;
   bool is_hit;
   bool is_solid;
   int8_t index;
   uint16_t* bitmap;
   void (*frame_iterate)(void* me);
}entity;

/*
typedef struct{
   
   
}colision_stat;
*/

//use framebuffer for now
static uint16_t *const vram = ((uint16_t*)0x06000000);
static uint16_t keys;

#define ENTITYS 20
#define PLAYER characters[0]

uint16_t* bitmap_conv_ram;//[SCREEN_WIDTH * SCREEN_HEIGHT];
uint16_t* background;//[SCREEN_WIDTH * SCREEN_HEIGHT];
uint16_t  playermap[16 * 16];
uint16_t  crosshair[16 * 16];
uint16_t  crosshair2[16 * 16];

entity   characters[ENTITYS];
uint8_t  num_active_characters;

entity& get_avail_entity(){
   for(uint8_t cnt = 0; cnt < ENTITYS; cnt++){
      if(!characters[cnt].active){
         return characters[cnt];
      }
   }
   print_bsod("Entity overflow!");
   while(1);
}

void reset_entity(entity& ent){
   ent.x = 0;
   ent.y = 0;
   ent.w = 0;
   ent.h = 0;
   ent.dirty.x = 0;
   ent.dirty.y = 0;
   ent.dirty.is_dirty = false;
   ent.accel_x = 0;
   ent.accel_y = 0;
   ent.angle = 0;
   ent.active = false;
   ent.bullet = false;
   ent.is_hit = false;
   ent.is_solid = false;
   ent.index  = -1;
   ent.bitmap = NULL;
   
   //callback
   ent.frame_iterate = NULL;
}

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

void invert_color(uint16_t* data, uint32_t size){
   for(uint32_t cnt = 0; cnt < size; cnt++){
      //only modify visible pixels
      if(data[cnt] & 0x8000){
         data[cnt] = (~data[cnt]) | 0x8000;
      }
   }
}

void restore_background(entity& ent){
   if(!ent.dirty.is_dirty)return;
   for(uint16_t yinc = 0; yinc < ent.h; yinc++){
      for(uint16_t xinc = 0; xinc < ent.w; xinc++){
         vram[ent.dirty.x + xinc + ((ent.dirty.y + yinc) * SCREEN_WIDTH)] = background[ent.dirty.x + xinc + ((ent.dirty.y + yinc) * SCREEN_WIDTH)];
      }
   }
   ent.dirty.is_dirty = false;
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
   ent.dirty.x = ent.x;
   ent.dirty.y = ent.y;
   ent.dirty.is_dirty = true;
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

void render_entitys(){
   for(uint8_t cnt = 0; cnt < ENTITYS; cnt++){
      if(characters[cnt].active){
         draw_entity(characters[cnt]);
      }
   }
}

void clear_dirty_entitys(){
   for(uint8_t cnt = 0; cnt < ENTITYS; cnt++){
      if(characters[cnt].dirty.is_dirty){
         restore_background(characters[cnt]);
      }
   }
}

bool collision_test(entity& chr1, entity& chr2){
   if (chr1.x < chr2.x + chr2.w &&
       chr1.x + chr1.w > chr2.x &&
       chr1.y < chr2.y + chr2.h &&
       chr1.h + chr1.y > chr2.y)
   {
      return true;
   }
   return false;
}

void test_collisions(){
   for(uint8_t cnt = 0; cnt < ENTITYS; cnt++){
      for(uint8_t cmp = cnt + 1; cmp < ENTITYS; cmp++){
         if(characters[cnt].active && characters[cmp].active){
            if(collision_test(characters[cnt], characters[cmp])){
               characters[cnt].is_hit = true;
               characters[cmp].is_hit = true;
            }
         }
      }
   }
}

void update_entitys(){
   for(uint8_t cnt = 0; cnt < ENTITYS; cnt++){
      if(characters[cnt].active && characters[cnt].frame_iterate){
         characters[cnt].frame_iterate(&characters[cnt]);
      }
   }
}

bool intersects_solid(entity& test){
   for(uint8_t cnt = 0; cnt < ENTITYS; cnt++){
      if(cnt != test.index/*prevent collide with self*/ && characters[cnt].active && characters[cnt].is_solid && collision_test(test, characters[cnt])){
         return true;
      }
   }
   return false;
}

void fire_gun(void* me){
   entity& this_ent = *((entity*)me);
   if(this_ent.is_hit){
      this_ent.bitmap = crosshair2;
      this_ent.is_hit = false;
   }
   else{
      this_ent.bitmap = crosshair;
   }
}

void move_player(void* me){
   entity& this_ent = *((entity*)me);
   
   if(keys & KEY_LEFT){
      if(this_ent.x > 0){
         this_ent.x--;
         //dont allow moving into solid objects
         if(intersects_solid(this_ent)){
            this_ent.x++;
         }
      }
   }
   else if(keys & KEY_RIGHT){
      if(this_ent.x + this_ent.w < SCREEN_WIDTH){
         this_ent.x++;
         //dont allow moving into solid objects
         if(intersects_solid(this_ent)){
            this_ent.x--;
         }
      }
   }
   
   if(keys & KEY_UP){
      if(this_ent.y > 0){
         this_ent.y--;
         //dont allow moving into solid objects
         if(intersects_solid(this_ent)){
            this_ent.y++;
         }
      }
   }
   else if(keys & KEY_DOWN){
      if(this_ent.y + this_ent.h < SCREEN_HEIGHT){
         this_ent.y++;
         //dont allow moving into solid objects
         if(intersects_solid(this_ent)){
            this_ent.y--;
         }
      }
   }
   
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
   
   entity logo;
   conv_32bpp_to_16(bitmap_conv_ram, (uint32_t*)clexa_logo_data[0], CLEXA_LOGO_FRAME_WIDTH * CLEXA_LOGO_FRAME_HEIGHT);
   //need to fix starting location
   logo.x = 70;
   logo.y = 10;
   logo.w = CLEXA_LOGO_FRAME_WIDTH;
   logo.h = CLEXA_LOGO_FRAME_HEIGHT;
   logo.bitmap = bitmap_conv_ram;
   logo.active = true;
   draw_entity_background(logo);
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
   
   
   conv_32bpp_to_16(playermap, (uint32_t*)clarke_data[0], 16 * 16);
   conv_32bpp_to_16(crosshair, (uint32_t*)crosshair_data[0], 16 * 16);
   
   //fired crosshair
   memcpy(crosshair2, crosshair, 16 * 16 * sizeof(uint16_t));
   invert_color(crosshair2, 16 * 16);
   
   for(uint8_t cnt = 0; cnt < ENTITYS; cnt++){
      reset_entity(characters[cnt]);
   }
   
   PLAYER.x = 0;
   PLAYER.y = 0;
   PLAYER.w = 16;
   PLAYER.h = 16;
   PLAYER.active = true;
   PLAYER.index = 0;
   PLAYER.bitmap = playermap;
   PLAYER.frame_iterate = move_player;
   
   entity& thing1 = get_avail_entity();
   reset_entity(thing1);
   thing1.x = 50;
   thing1.y = 50;
   thing1.w = 16;
   thing1.h = 16;
   thing1.active = true;
   thing1.is_solid = true;//just a test
   thing1.index  = 1;
   thing1.bitmap = crosshair;
   thing1.frame_iterate = fire_gun;
}

void switch_to_game(){
   for(uint32_t cnt = 0; cnt < SCREEN_WIDTH * SCREEN_HEIGHT; cnt++){
      vram[cnt] = background[cnt];
   }
}

void run_frame_game(){
   keys = ~(REG_KEYINPUT);
   
   if(keys & KEY_B){
      draw_logo();
   }

   test_collisions();
   update_entitys();
   clear_dirty_entitys();
   render_entitys();
}
