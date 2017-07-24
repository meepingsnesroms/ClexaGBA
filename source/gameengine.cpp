#include <gba_video.h>
#include <gba_input.h>
#include <gba_systemcalls.h>

#include <stdlib.h>
#include <string.h>

#include "assets.h"

#include "uguishim.h"
#include "inventory.h"
#include "speedhacks.h"
#include "gametypes.h"
#include "rendering.h"
#include "diag.h"
#include "trig.h"
#include "macro.h"
#include "timer.h"
#include "rng.h"

//level testing
#define MAX_LEVELS 20

//#include "../data/levels/level1.c.out"
#include "../data/levels/level2.c.out"
#include "../data/leveltest.c.out"

level* level_nums[MAX_LEVELS];

#define TEST_ENTITYS 2

entity level_test_entitys[TEST_ENTITYS];

//level lvl1 = {0/*id*/, 0/*north*/, 0/*south*/, 0/*east*/, 0/*west*/, NULL/*sprites*/, 0/*num_sprites*/, title_screen_data/*background*/, level1_data/*foreground*/};
level lvl1 = {0/*id*/, 0/*north*/, 0/*south*/, 1/*east*/, 0/*west*/, level_test_entitys/*sprites*/, TEST_ENTITYS/*num_sprites*/, title_screen_data/*background*/, leveltest_data/*foreground*/};
level lvl2 = {0/*id*/, 0/*north*/, 0/*south*/, 0/*east*/, 0/*west*/, level_test_entitys/*sprites*/, TEST_ENTITYS/*num_sprites*/, title_screen_data/*background*/, level2_data/*foreground*/};

//end of level testing

#define ASTROID_SPAWN_PROBABILITY 10 //probability is 1/ASTROID_SPAWN_PROBABILITY
#define ASTEROID_SPEED 2.0
#define MAX_ASTEROIDS 5
#define CROSSHAIR_DISTANCE 20.0
#define BULLET_SPEED 3.0
#define BULLET_COLOR 0xFEFE
#define MAX_ENTITYS 20
#define PLAYER characters[0]

static level*   current_level;
static entity   characters[MAX_ENTITYS];
static uint8_t* enviroment_map;//map of terrain type

static uint16_t keys;
static timer    play_time;
static bool     have_nightblood;
static int32_t  active_asteroids;
static bool     fire_pressed_last_frame;

entity& get_avail_entity(){
   for(int32_t cnt = 0; cnt < MAX_ENTITYS; cnt++){
      if(!characters[cnt].active){
         characters[cnt].index = cnt;//keep the index current
         return characters[cnt];
      }
   }
   bsod("Entity overflow!");
   return MAKE_NULL_OBJECT(entity);//this is for the compiler warning only, there is no coming back after bsod
}

inline void set_environ_data(int32_t x, int32_t y, uint8_t data){
   enviroment_map[y * SCREEN_WIDTH + x] = data;
}

inline uint8_t get_environ_data(int32_t x, int32_t y){
   return enviroment_map[y * SCREEN_WIDTH + x];
}

void border_wall(){
   for(int32_t cnt = 0; cnt < SCREEN_WIDTH; cnt++){
      set_environ_data(cnt, 0, 0x01);
      set_environ_data(cnt, SCREEN_HEIGHT - 1, 0x01);
   }
   
   for(int32_t cnt = 0; cnt < SCREEN_HEIGHT; cnt++){
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
   ent.use_fixedpt = false;
   ent.is_enemy = false;
   ent.is_solid = false;
   ent.health = 100;
   //ent.index  = -1;//the index must remain valid for entitys in the characters array
   ent.sprite_x_offset = 0;
   ent.sprite_y_offset = 0;
   ent.sprite = {0, 0, NULL};
   ent.data   = NULL;
   
   //callback
   ent.frame_iterate = NULL;
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
   for(int32_t cnt = 0; cnt < MAX_ENTITYS; cnt++){
      if(characters[cnt].active && characters[cnt].sprite.bitmap != NULL){
         draw_entity(characters[cnt]);
      }
   }
}

void clear_dirty_entitys(){
   for(int32_t cnt = 0; cnt < MAX_ENTITYS; cnt++){
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
   //the background was just drawn so there are no dirty segments left
   for(int32_t cnt = 0; cnt < MAX_ENTITYS; cnt++){
      characters[cnt].dirty.is_dirty = false;
   }
   render_entitys();
}

bool collision_inside(entity& chr1, entity& chr2){
   //this function tests if the center of obj1 is in the bounding box of obj2
   //or if the center of obj2 is in the bounding box of obj1
   int32_t mid_x_1 = chr1.x + (chr1.w / 2);
   int32_t mid_y_1 = chr1.y + (chr1.h / 2);
   
   int32_t mid_x_2 = chr2.x + (chr2.w / 2);
   int32_t mid_y_2 = chr2.y + (chr2.h / 2);
   
   if (chr1.x <= mid_x_2 &&
       chr1.x + chr1.w >= mid_x_2 &&
       chr1.y <= mid_y_2 &&
       chr1.h + chr1.y >= mid_y_2)
   {
      return true;
   }
   
   if (chr2.x <= mid_x_1 &&
       chr2.x + chr2.w >= mid_x_1 &&
       chr2.y <= mid_y_1 &&
       chr2.h + chr2.y >= mid_y_1)
   {
      return true;
   }
   
   return false;
}

bool collision_touching(entity& chr1, entity& chr2){
   if (chr1.x < chr2.x + chr2.w &&
       chr1.x + chr1.w > chr2.x &&
       chr1.y < chr2.y + chr2.h &&
       chr1.h + chr1.y > chr2.y)
   {
      return true;
   }
   return false;
}

bool collision_test_point(entity& chr1, int32_t x, int32_t y){
   if (chr1.x <= x &&
       chr1.x + chr1.w >= x &&
       chr1.y <= y &&
       chr1.h + chr1.y >= y)
   {
      return true;
   }
   return false;
}

void update_entitys(){
   for(int32_t cnt = 0; cnt < MAX_ENTITYS; cnt++){
      if(characters[cnt].active && characters[cnt].frame_iterate != NULL){
         characters[cnt].frame_iterate(&characters[cnt]);
      }
   }
}

bool intersects_solid(entity& test){
   for(int32_t cnt = 0; cnt < MAX_ENTITYS; cnt++){
      if(cnt != test.index/*prevent collide with self*/ && characters[cnt].active && characters[cnt].is_solid && collision_inside(test, characters[cnt])){
         return true;
      }
   }
   return false;
}

void change_level(level* new_level, uint8_t direction){
   //background
   memcpy32(background, new_level->background, SCREEN_WIDTH * SCREEN_HEIGHT / 2);
   
   //foreground
   texture foreground_wrapper = {SCREEN_WIDTH, SCREEN_HEIGHT, new_level->foreground};
   draw_texture_background(0, 0, foreground_wrapper);//the foreground is put on the background bitmap to prevent entitys from overwriteing it
   
   //terrain
   conv_16bpp_to_terrain(enviroment_map, new_level->foreground, SCREEN_WIDTH * SCREEN_HEIGHT);
   
   //add invisible border wall, yes, trump style
   border_wall();
   
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
   for(int32_t cnt = 0; cnt < MAX_ENTITYS; cnt++){
      if(characters[cnt].kill_on_exit){
         characters[cnt].active = false;
      }
   }
   
   //add levels new entitys
   for(int32_t cnt = 0; cnt < new_level->num_sprites; cnt++){
      entity& new_sprite  = get_avail_entity();
      int32_t old_index = new_sprite.index;
      
      new_sprite = new_level->sprites[cnt];
      new_sprite.index = old_index;
   }
   
   //clear outdated framebuffer
   redraw_screen();
   
   current_level = new_level;
}

void level_teleporter(void* me){
   entity& this_ent = REFERENCE_FROM_PTR(entity, me);
   
   if(collision_touching(PLAYER, this_ent)){
      switch(this_ent.angle){
         case 0://up
            change_level(level_nums[current_level->north], DIR_UP);
            break;
            
         case 90://right
            change_level(level_nums[current_level->east], DIR_RIGHT);
            break;
            
         case 180://down
            change_level(level_nums[current_level->south], DIR_DOWN);
            break;
            
         case 270://left
            change_level(level_nums[current_level->west], DIR_LEFT);
            break;
            
         default:
            gba_printf("Angle:%d is not a cardinal direction!", this_ent.angle);
            bsod("Invalid angle parameter!");
            break;
      }
   }
}

void item_collect(void* me){
   entity& this_ent = REFERENCE_FROM_PTR(entity, me);
   if(collision_inside(PLAYER, this_ent)){
      
      //add item to inventory
      add_item(REFERENCE_FROM_PTR(item, this_ent.data));
      
      //destroy the item entity
      this_ent.active = false;
   }
}

void move_bullet(void* me){
   entity& this_ent = REFERENCE_FROM_PTR(entity, me);
   
   //bullets use fixed point stored in existing x/y and accel_x/y varibles
   
   restore_background_pixel(this_ent.dirty.x, this_ent.dirty.y);
   
   this_ent.fxd_x = fixedpt_add(this_ent.fxd_x, this_ent.fxd_accel_x);
   this_ent.fxd_y = fixedpt_add(this_ent.fxd_y, this_ent.fxd_accel_y);
   
   int32_t x_as_int = fixedpt_toint(this_ent.fxd_x);
   int32_t y_as_int = fixedpt_toint(this_ent.fxd_y);
   
   //test bullet validity
   if(x_as_int < 0 || y_as_int < 0 || x_as_int > SCREEN_WIDTH - 1 || y_as_int > SCREEN_HEIGHT - 1){
      //bullet invalid due to being off screen
      this_ent.active = false;
   }
   else{
      //redraw bullet
      plot_vram_pixel(x_as_int, y_as_int, BULLET_COLOR);
      this_ent.dirty.x = x_as_int;
      this_ent.dirty.y = y_as_int;
   }
}

void gun_crosshair(void* me){
   entity& this_ent = REFERENCE_FROM_PTR(entity, me);
   
   if(this_ent.angle != PLAYER.angle){
      //pick a direction to move
      if(PLAYER.angle - this_ent.angle > -3 && PLAYER.angle - this_ent.angle < 3){
         //if within 6 of target angle clamp to target
         this_ent.angle = PLAYER.angle;
      }
      else if(PLAYER.angle == 270 && this_ent.angle < 90){
         this_ent.angle -= 3;
      }
      else if(PLAYER.angle > this_ent.angle){
         this_ent.angle += 3;
      }
      else if(PLAYER.angle == 0 && this_ent.angle > 180){
         this_ent.angle += 3;
      }
      else if(PLAYER.angle < this_ent.angle){
         this_ent.angle -= 3;
      }
      
      if(this_ent.angle < 0)this_ent.angle += 360;
      if(this_ent.angle >= 360)this_ent.angle -= 360;
   }
   
   int32_t player_mid_x = PLAYER.x + (PLAYER.w / 2);
   int32_t player_mid_y = PLAYER.y + (PLAYER.h / 2);
   
   fixedpt circle_radius = fixedpt_rconst(CROSSHAIR_DISTANCE);
   fixedpt x_offset = fixedpt_mul(circle_radius, fixedpt_cos_deg(fixedpt_fromint(this_ent.angle - 90)));
   fixedpt y_offset = fixedpt_mul(circle_radius, fixedpt_sin_deg(fixedpt_fromint(this_ent.angle - 90)));
   
   this_ent.x = fixedpt_toint(x_offset) + player_mid_x;
   this_ent.y = fixedpt_toint(y_offset) + player_mid_y;
   
   if(!fire_pressed_last_frame && (keys & KEY_B)){
      entity& new_bullet = get_avail_entity();
      reset_entity(new_bullet);
      
      //use fixedpt for bullets
      new_bullet.fxd_x = fixedpt_fromint((this_ent.x + player_mid_x) / 2);//half way between player and crosshair
      new_bullet.fxd_y = fixedpt_fromint((this_ent.y + player_mid_y) / 2);//half way between player and crosshair
      
      //need to calculate these from angle
      new_bullet.fxd_accel_x = fixedpt_mul(fixedpt_rconst(BULLET_SPEED), fixedpt_cos_deg(fixedpt_fromint(this_ent.angle - 90)));
      new_bullet.fxd_accel_y = fixedpt_mul(fixedpt_rconst(BULLET_SPEED), fixedpt_sin_deg(fixedpt_fromint(this_ent.angle - 90)));
      
      //bullets are always 1x1 in size
      //new_bullet.w = 1;
      //new_bullet.h = 1;
      
      //new_bullet.dirty.x = 0;//set by reset_entity(new_bullet);
      //new_bullet.dirty.y = 0;//set by reset_entity(new_bullet);
      new_bullet.dirty.is_dirty = false;//the dirty.x/y are used but should not be cleared by the clear_dirty_entitys() routine
      new_bullet.angle = this_ent.angle;
      new_bullet.active = true;
      new_bullet.kill_on_exit = true;
      new_bullet.use_fixedpt = true;
      new_bullet.frame_iterate = move_bullet;
   }
}

void move_asteroid(void* me){
   //uses fixedpt
   entity& this_ent = REFERENCE_FROM_PTR(entity, me);
   
   //asteroids use fixed point stored in existing x/y and accel_x/y varibles
   
   //restore_background_pixel(this_ent.dirty.x, this_ent.dirty.y);
   
   this_ent.fxd_x = fixedpt_add(this_ent.fxd_x, this_ent.fxd_accel_x);
   this_ent.fxd_y = fixedpt_add(this_ent.fxd_y, this_ent.fxd_accel_y);
   
   int32_t x_as_int = fixedpt_toint(this_ent.fxd_x);
   int32_t y_as_int = fixedpt_toint(this_ent.fxd_y);
   
   //test asteroid validity
   if(x_as_int < 0 || y_as_int < 0 || x_as_int > SCREEN_WIDTH - 1 || y_as_int > SCREEN_HEIGHT - 1){
      //asteroid invalid due to being off screen
      this_ent.active = false;
      active_asteroids--;
   }
   /*
   else{
      //redraw asteroid
      plot_vram_pixel(x_as_int, y_as_int, BULLET_COLOR);
      this_ent.dirty.x = x_as_int;
      this_ent.dirty.y = y_as_int;
   }
   */
}

void asteroid_spawner(void* me){
   bool spawn_asteroid = ((rng_rand() % ASTROID_SPAWN_PROBABILITY) == 1);
   
   if(spawn_asteroid && active_asteroids < MAX_ASTEROIDS){
      int32_t x_coord   = rng_rand() % SCREEN_WIDTH;
      int32_t rnd_angle = (rng_rand() % 180) + 90;
      
      entity& new_asteroid = get_avail_entity();
      reset_entity(new_asteroid);
      
      //use fixedpt for asteroids
      new_asteroid.fxd_x = fixedpt_fromint(x_coord);
      new_asteroid.fxd_y = fixedpt_fromint(0);//y is always 0 because asteroids come from above
      
      //need to calculate these from angle
      new_asteroid.fxd_accel_x = fixedpt_mul(fixedpt_rconst(ASTEROID_SPEED), fixedpt_cos_deg(fixedpt_fromint(rnd_angle - 90)));
      new_asteroid.fxd_accel_y = fixedpt_mul(fixedpt_rconst(ASTEROID_SPEED), fixedpt_sin_deg(fixedpt_fromint(rnd_angle - 90)));
      
      new_asteroid.w = 16;
      new_asteroid.h = 16;
      
      new_asteroid.angle = rnd_angle;
      new_asteroid.active = true;
      new_asteroid.kill_on_exit = true;
      new_asteroid.use_fixedpt = true;
      new_asteroid.sprite = rock_tex;
      new_asteroid.frame_iterate = move_asteroid;
      
      active_asteroids++;
   }
}

void move_player(void* me){
   entity& this_ent = REFERENCE_FROM_PTR(entity, me);
   
   if(keys & KEY_L){
      open_inventory();
      redraw_screen();//the item list corrupts the vram so a full redraw is needed
   }
   
   if(keys & KEY_LEFT){
      this_ent.accel_x--;
      this_ent.angle = 270;
      this_ent.sprite.bitmap = clarke_left_data;
   }
   else if(keys & KEY_RIGHT){
      this_ent.accel_x++;
      this_ent.angle = 90;
      this_ent.sprite.bitmap = clarke_right_data;
   }
   
   if(keys & KEY_UP){
      //cant fly
      this_ent.accel_y--;
      this_ent.angle = 0;
      this_ent.sprite.bitmap = clarke_back_data;
   }
   else if(keys & KEY_DOWN){
      this_ent.accel_y++;
      this_ent.angle = 180;
      this_ent.sprite.bitmap = clarke_front_data;
   }
   
   
   //quick version, works only if there are no deep concave objects
   if(this_ent.accel_x != 0 || this_ent.accel_y != 0){
      int32_t mid_x = this_ent.x + (this_ent.w / 2);
      int32_t mid_y = this_ent.y + (this_ent.h / 2);
      
      if(get_environ_data(mid_x + this_ent.accel_x, mid_y) == 0x00){
         this_ent.x += this_ent.accel_x;
         mid_x = this_ent.x + (this_ent.w / 2);//needs to be updated after change
      }
      
      if(get_environ_data(mid_x, mid_y + this_ent.accel_y) == 0x00){
         this_ent.y += this_ent.accel_y;
      }

      this_ent.accel_x = 0;
      this_ent.accel_y = 0;
   }
   
}

static void clear_globals(){
   //effects health bar rendering
   have_nightblood = false;
   
   //prevents the gun from firing 60 rounds per second(the gun is a police pistol not AR-15!)
   fire_pressed_last_frame = false;
   
   //asteroids on screen
   active_asteroids = 0;
   
   //game timer
   timer_reset(&play_time);
}

void init_game(){
   enviroment_map = (uint8_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT);
   
   if(enviroment_map == NULL){
      bsod("Not enough memory!");
   }
   
   //init music
   //not done yet
   
   for(int32_t cnt = 0; cnt < MAX_ENTITYS; cnt++){
      reset_entity(characters[cnt]);
      characters[cnt].index = cnt;
   }
   
   
   
   //level testing
   for(int32_t cnt = 0; cnt < MAX_LEVELS; cnt++){
      level_nums[cnt] = NULL;
   }
   
   level_nums[0] = &lvl1;
   level_nums[1] = &lvl2;
   
   //end of level testing
   
   
   
   clear_globals();
   
   PLAYER.x = SCREEN_WIDTH  / 2;
   PLAYER.y = SCREEN_HEIGHT / 2;
   PLAYER.w = 15 - 4;
   PLAYER.h = 16 - 4;
   PLAYER.active = true;
   PLAYER.kill_on_exit = false;
   PLAYER.sprite_x_offset = -2;
   PLAYER.sprite_y_offset = -2;
   PLAYER.sprite = {16, 16, clarke_front_data};
   PLAYER.frame_iterate = move_player;
   
   entity& clarke_gun = get_avail_entity();
   reset_entity(clarke_gun);
   //x and y are set on first callback run
   clarke_gun.w = 7;
   clarke_gun.h = 7;
   //angle is set on first callback run
   clarke_gun.active = true;
   clarke_gun.kill_on_exit = false;
   clarke_gun.is_solid = false;
   clarke_gun.sprite_x_offset = -3;
   clarke_gun.sprite_y_offset = -3;
   clarke_gun.sprite = {7, 7, smallcrosshair_data};
   clarke_gun.frame_iterate = gun_crosshair;
   
   //level teleporter test
   reset_entity(level_test_entitys[0]);
   level_test_entitys[0].x = 200;
   level_test_entitys[0].y = 80;
   level_test_entitys[0].w = 16;
   level_test_entitys[0].h = 16;
   level_test_entitys[0].angle = 90;//determines what level is telepoted to
   level_test_entitys[0].active = true;
   level_test_entitys[0].kill_on_exit = true;
   level_test_entitys[0].is_solid = false;
   level_test_entitys[0].sprite = {16, 16, crosshair_data};
   level_test_entitys[0].frame_iterate = level_teleporter;
   
   //asteroid spawning test
   reset_entity(level_test_entitys[1]);
   level_test_entitys[1].frame_iterate = asteroid_spawner;
   level_test_entitys[1].active = true;
}

void switch_to_game(){
   //load first level
   change_level(&lvl1, DIR_NONE);
   
   draw_background();
   update_health_bar();
}

void run_frame_game(){
   keys = ~(REG_KEYINPUT);
   
   update_entitys();
   clear_dirty_entitys();
   render_entitys();
   update_health_bar();
   timer_tick(&play_time);
   
   fire_pressed_last_frame = (keys & KEY_B) ? true : false;
}
