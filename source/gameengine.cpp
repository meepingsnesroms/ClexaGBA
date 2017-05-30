#include <gba_video.h>
#include <gba_input.h>
#include <gba_systemcalls.h>

#include <stdlib.h>
#include <string.h>

#include "assets.h"

#include "../data/clexalogo.cdata"
#include "../data/crosshair.cdata"
#include "../data/polis.cdata"
#include "../data/leveltest.cdata"

#include "uguishim.h"
#include "inventory.h"
#include "speedhacks.h"
#include "gametypes.h"
#include "rendering.h"

static uint16_t keys;

#define ENTITYS 20
#define PLAYER characters[0]


uint16_t* bitmap_conv_ram;//[SCREEN_WIDTH * SCREEN_HEIGHT];
uint16_t  crosshair[16 * 16];
uint16_t  crosshair2[16 * 16];

entity   characters[ENTITYS];
uint8_t* enviroment_map;//map of terrain type
uint8_t  num_active_characters;

level* current_level;

bool have_nightblood;

entity& get_avail_entity(){
   for(uint8_t cnt = 0; cnt < ENTITYS; cnt++){
      if(!characters[cnt].active){
         return characters[cnt];
      }
   }
   bsod("Entity overflow!");
   return PLAYER;//this is for the compiler warning only, there is no coming back after bsod
}

inline void set_environ_data(uint16_t x, uint16_t y, uint8_t data){
   uint32_t offset = y * SCREEN_WIDTH + x;
   enviroment_map[offset] = data;
}

inline uint8_t get_environ_data(uint16_t x, uint16_t y){
   uint32_t offset = y * SCREEN_WIDTH + x;
   return enviroment_map[offset];
}

void border_wall(){
   for(uint16_t cnt = 0; cnt < SCREEN_WIDTH; cnt++){
      set_environ_data(cnt, 0, 0x01);
      set_environ_data(cnt, SCREEN_HEIGHT - 1, 0x01);
   }
   
   for(uint16_t cnt = 0; cnt < SCREEN_HEIGHT; cnt++){
      set_environ_data(0, cnt, 0x01);
      set_environ_data(SCREEN_WIDTH - 1, cnt, 0x01);
   }
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
   ent.gravity = 0;
   ent.active = false;
   ent.kill_on_exit = true;
   ent.bullet = false;
   ent.is_hit = false;
   ent.is_solid = false;
   ent.health = 100;
   ent.index  = -1;
   ent.sprite_x_offset = 0;
   ent.sprite_y_offset = 0;
   ent.sprite = {0, 0, NULL};
   
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

void conv_32bpp_to_terrain(uint8_t* output, uint32_t* data, uint32_t size){
   for(uint32_t cnt = 0; cnt < size; cnt++){
      //output[cnt] = data[cnt] & 0xFF;
      output[cnt] = data[cnt] & 0x01;
   }
}

void conv_16bpp_to_terrain(uint8_t* output, uint16_t* data, uint32_t size){
   for(uint32_t cnt = 0; cnt < size; cnt++){
      if(data[cnt] & 0x8000){
         output[cnt] = 0x01;
      }
      else {
         output[cnt] = 0x00;
      }
   }
}

void change_level(level* new_level, uint8_t direction){
   //background
   memcpy32(background, new_level->background, SCREEN_WIDTH * SCREEN_HEIGHT / 2);
   
   //foreground
   texture foreground_wrapper = {SCREEN_WIDTH, SCREEN_HEIGHT, new_level->foreground};
   draw_texture(0, 0, foreground_wrapper);
   
   //terrain
   conv_16bpp_to_terrain(enviroment_map, new_level->foreground, SCREEN_WIDTH * SCREEN_HEIGHT);
   
   //move player
   switch(direction){
      case DIR_UP:
         PLAYER.y = SCREEN_HEIGHT - 1 - PLAYER.h;//put player on bottom of new screen
         break;
      case DIR_DOWN:
         PLAYER.y = 0;//put player on top of new screen
         break;
      case DIR_LEFT:
         PLAYER.x = SCREEN_WIDTH - 1 - PLAYER.w;//put player on right side of new screen
         break;
      case DIR_RIGHT:
         PLAYER.x = 0;//put player on left side of new screen
         break;
      case DIR_NONE:
         //put player in center of screen when starting
         PLAYER.x = (SCREEN_WIDTH / 2) - (PLAYER.w / 2);
         PLAYER.y = (SCREEN_HEIGHT / 2) - (PLAYER.h / 2);
         break;
         
      default:
         bsod("invalid derection parameter");
         break;
   }
   
   //clear entitys
   for(uint8_t cnt = 0; cnt < ENTITYS; cnt++){
      if(characters[cnt].kill_on_exit){
         characters[cnt].active = false;
      }
   }
   
   current_level = new_level;
}

bool crossed_border(entity& ent){
   if(ent.x < 0 || ent.y < 0){
      return true;
   }
   if(ent.x + ent.w > SCREEN_WIDTH - 1 || ent.y + ent.h > SCREEN_HEIGHT - 1){
      return true;
   }
   return false;
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

void update_health_bar(){
   restore_background(2, 2, 100 + 2 + 1, 5 + 2 + 1);
   UG_DrawRoundFrame(2, 2, 2 + 100 + 2, 2 + 5 + 2, 3, C_GOLDEN_ROD);
   UG_FillFrame(3, 3, 3 + PLAYER.health, 3 + 5, have_nightblood ? C_BLACK : C_DARK_RED);//maybe C_DARK_GRAY instead of C_BLACK
}

void redraw_screen(){
   //draw background
   draw_background();
   
   //draw entitys
   clear_dirty_entitys();
   render_entitys();
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

bool collision_test_point(entity& chr1, uint16_t x, uint16_t y){
   if (chr1.x <= x &&
       chr1.x + chr1.w >= x &&
       chr1.y <= y &&
       chr1.h + chr1.y >= y)
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
      this_ent.sprite.bitmap = crosshair2;
      this_ent.is_hit = false;
   }
   else{
      this_ent.sprite.bitmap = crosshair;
   }
}

void move_player(void* me){
   entity& this_ent = *((entity*)me);
   
   if(keys & KEY_L){
      open_inventory();
      redraw_screen();//the item list corrupts the vram so a full redraw is needed
   }
   
   
   //debug only!
   if(keys & KEY_R){
      have_nightblood = true;
      update_health_bar();
   }
   
   
   if(keys & KEY_LEFT){
      this_ent.accel_x--;
      this_ent.sprite.bitmap = clarke_left_data;
   }
   else if(keys & KEY_RIGHT){
      this_ent.accel_x++;
      this_ent.sprite.bitmap = clarke_right_data;
   }
   
   if(keys & KEY_UP){
      //cant fly
      this_ent.accel_y--;
      this_ent.sprite.bitmap = clarke_back_data;
   }
   else if(keys & KEY_DOWN){
      this_ent.accel_y++;
      this_ent.sprite.bitmap = clarke_front_data;
   }
   
   
   if(this_ent.accel_x > 0){
      for(int8_t scoot = 0; scoot < this_ent.accel_x; scoot++){
         if(get_environ_data(this_ent.x + this_ent.w/* + 1*/, this_ent.y) != 0x00){
            break;//cant walk into a wall
         }
         if(get_environ_data(this_ent.x + this_ent.w/* + 1*/, this_ent.y + this_ent.h / 2) != 0x00){
            break;//cant walk into a wall
         }
         if(get_environ_data(this_ent.x + this_ent.w/* + 1*/, this_ent.y + this_ent.h - 1) != 0x00){
            break;//cant walk into a wall
         }
         this_ent.x++;
      }
   }
   else if(this_ent.accel_x < 0){
      for(int8_t scoot = 0; scoot > this_ent.accel_x; scoot--){
         if(get_environ_data(this_ent.x - 1, this_ent.y) != 0x00){
            break;//cant walk into a wall
         }
         if(get_environ_data(this_ent.x - 1, this_ent.y + this_ent.h / 2) != 0x00){
            break;//cant walk into a wall
         }
         if(get_environ_data(this_ent.x - 1, this_ent.y + this_ent.h - 1) != 0x00){
            break;//cant walk into a wall
         }
         this_ent.x--;
      }
   }
   this_ent.accel_x = 0;
   
   if(this_ent.accel_y > 0){
      for(int8_t scoot = 0; scoot < this_ent.accel_y; scoot++){
         if(get_environ_data(this_ent.x, this_ent.y + this_ent.h/* + 1*/) != 0x00){
            break;//cant walk into a wall
         }
         if(get_environ_data(this_ent.x + this_ent.w / 2, this_ent.y + this_ent.h/* + 1*/) != 0x00){
            break;//cant walk into a wall
         }
         if(get_environ_data(this_ent.x + this_ent.w - 1, this_ent.y + this_ent.h/* + 1*/) != 0x00){
            break;//cant walk into a wall
         }
         this_ent.y++;
      }
   }
   else if(this_ent.accel_y < 0){
      for(int8_t scoot = 0; scoot > this_ent.accel_y; scoot--){
         if(get_environ_data(this_ent.x, this_ent.y - 1) != 0x00){
            break;//cant walk into a wall
         }
         if(get_environ_data(this_ent.x + this_ent.w / 2, this_ent.y - 1) != 0x00){
            break;//cant walk into a wall
         }
         if(get_environ_data(this_ent.x + this_ent.w - 1, this_ent.y - 1) != 0x00){
            break;//cant walk into a wall
         }
         this_ent.y--;
      }
   }
   this_ent.accel_y = 0;
   
}

void draw_logo(){
   entity polis;
   reset_entity(polis);
   conv_32bpp_to_16(bitmap_conv_ram, (uint32_t*)polis_data[0], POLIS_FRAME_WIDTH * POLIS_FRAME_HEIGHT);
   polis.x = 0;
   polis.y = 15;//gba is 160 tall, image is 135
   polis.w = POLIS_FRAME_WIDTH;
   polis.h = POLIS_FRAME_HEIGHT;
   polis.sprite = {POLIS_FRAME_WIDTH, POLIS_FRAME_HEIGHT, bitmap_conv_ram};
   polis.active = true;
   draw_entity_background(polis);
   
   /*
   entity logo;
   reset_entity(logo);
   conv_32bpp_to_16(bitmap_conv_ram, (uint32_t*)clexa_logo_data[0], CLEXA_LOGO_FRAME_WIDTH * CLEXA_LOGO_FRAME_HEIGHT);
   logo.x = 70;
   logo.y = 0;
   logo.w = CLEXA_LOGO_FRAME_WIDTH;
   logo.h = CLEXA_LOGO_FRAME_HEIGHT;
   logo.sprite = {CLEXA_LOGO_FRAME_WIDTH, CLEXA_LOGO_FRAME_HEIGHT, bitmap_conv_ram};
   logo.active = true;
   draw_entity_background(logo);
   */
}

void init_game(){
   enviroment_map    = (uint8_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT);
   bitmap_conv_ram   = (uint16_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));
   
   if(bitmap_conv_ram == NULL || enviroment_map == NULL){
      bsod("Not enough memory!");
   }
   
   conv_32bpp_to_terrain(enviroment_map, (uint32_t*)leveltest_data[0], SCREEN_WIDTH * SCREEN_HEIGHT);
   
   //init music
   //not done yet
   
   conv_32bpp_to_16(crosshair, (uint32_t*)crosshair_data[0], 16 * 16);
   
   //fired crosshair
   memcpy(crosshair2, crosshair, 16 * 16 * sizeof(uint16_t));
   invert_color(crosshair2, 16 * 16);
   
   for(uint8_t cnt = 0; cnt < ENTITYS; cnt++){
      reset_entity(characters[cnt]);
   }
   
   //effects health bar rendering
   have_nightblood = false;
   
   PLAYER.x = SCREEN_WIDTH  / 2;
   PLAYER.y = SCREEN_HEIGHT / 2;
   PLAYER.w = 15 - 4;
   PLAYER.h = 16 - 4;
   PLAYER.active = true;
   PLAYER.kill_on_exit = false;
   PLAYER.index = 0;
   PLAYER.sprite_x_offset = -2;
   PLAYER.sprite_y_offset = -2;
   PLAYER.sprite = {16, 16, clarke_front_data};
   PLAYER.frame_iterate = move_player;
   
   entity& thing1 = get_avail_entity();
   reset_entity(thing1);
   thing1.x = 50;
   thing1.y = 50;
   thing1.w = 16;
   thing1.h = 16;
   thing1.active = true;
   thing1.kill_on_exit = false;//temp, just for testing
   thing1.is_solid = true;//just a test
   thing1.index  = 1;
   thing1.sprite = {16, 16, crosshair};
   thing1.frame_iterate = fire_gun;
}

void switch_to_game(){
   //fill to black
   fill_background(0x0000);
   
   //polis tower
   draw_logo();
   
   //prevents leaving the screen
   border_wall();
   
   //draw ground
   for(uint16_t inc_y = 0; inc_y < SCREEN_HEIGHT; inc_y++){
      for(uint16_t inc_x = 0; inc_x < SCREEN_WIDTH; inc_x++){
         switch(get_environ_data(inc_x, inc_y)){
            case 0x00:
               break;
            case 0x01:
               background[inc_y * SCREEN_WIDTH + inc_x] = get_texture_pixel(inc_x, inc_y, rock_tex);
               break;
         }
      }
   }
   
   draw_background();
   update_health_bar();
}

void run_frame_game(){
   keys = ~(REG_KEYINPUT);

   test_collisions();
   update_entitys();
   clear_dirty_entitys();
   render_entitys();
   update_health_bar();
}
