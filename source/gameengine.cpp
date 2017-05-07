#include <gba_video.h>
#include <vector>

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

std::vector<entity> characters;

bool collision_test(entity& chr1, entity& chr2){
   
}

/*
entity   characters[50];
entity*  active_characters[50];
uint8_t  num_active_characters;

void cull_characters(){
   
}
*/

void init_game(){
   
   //SetMode(MODE_3 | BG2_ON /* 240*160 16bit color */);
}

void switch_to_game(){
   
}

void run_frame_game(){
   
}
