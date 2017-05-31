#include <gba_video.h>//for SCREEN_* macros
#include <gba_input.h>

#include "uguishim.h"
#include "runloop.h"
#include "gameengine.h"
#include "itemmenu.h"
#include "diag.h"
#include "audio.h"
#include "assets.h"
#include "rendering.h"

#define SELECTOR_COLOR C_GOLDEN_ROD
#define NORMAL_COLOR   C_BLACK
#define LIST_START_X   50
#define LIST_START_Y   50
#define SPACE_BETWEEN_ENTRYS 12
#define ENTRY_WIDTH          50
#define ENTRY_HEIGHT         10
#define CORNER_RADIUS        3

static uint8_t selected_option;
static uint8_t total_items = 2;
static char    option_names[2][20] = {"Start", "Options"};

static void render_menu(bool first_render){
   if(first_render){
      texture tmp = {240, 160, title_screen_data};
      draw_texture_background(0, 0, tmp);
      draw_background();
   }
   
   UG_SetForecolor(C_WHITE);//use white text
   
   uint16_t y_offset = LIST_START_Y;
   for(uint8_t count = 0; count < total_items; count++){
      if(count == selected_option){
         //draw special color
         UG_FillRoundFrame(LIST_START_X, y_offset, LIST_START_X + ENTRY_WIDTH - 1, y_offset + ENTRY_HEIGHT - 1, CORNER_RADIUS, SELECTOR_COLOR);
         UG_SetBackcolor(SELECTOR_COLOR);
         UG_PutString(LIST_START_X + 1, y_offset + 1, option_names[count]);
      }
      else{
         //draw normal color
         UG_FillRoundFrame(LIST_START_X, y_offset, LIST_START_X + ENTRY_WIDTH - 1, y_offset + ENTRY_HEIGHT - 1, CORNER_RADIUS, NORMAL_COLOR);
         UG_SetBackcolor(NORMAL_COLOR);
         UG_PutString(LIST_START_X + 1, y_offset + 1, option_names[count]);
      }
      y_offset += SPACE_BETWEEN_ENTRYS;
   }
   
}

void init_menu(){
   selected_option = 0;
}

void switch_to_menu(){
   //SetMode(MODE_3 | BG2_ON /* 240*160 16bit color */);
   selected_option = 0;
   render_menu(true);
}

void draw_menu(){
   bool update_window = false;
   uint16_t keys = keysDown();
   
   if(keys & KEY_UP){
      if(selected_option > 0){
         selected_option--;
         update_window = true;
      }
   }
   
   if(keys & KEY_DOWN){
      if(selected_option < total_items - 1){
         selected_option++;
         update_window = true;
      }
   }
   
   //select button
   if(keys & KEY_A){
      switch(selected_option){
         case 0:
            //Start button
            in_game = true;
            update_window = false;
            switch_to_game();
            break;
         case 1:
            //Options button
            //no options yet
            break;
         default:
            //do nothing
            break;
      }
   }
   
   //run tests
   if(keys & KEY_L){
      play_test();//audio test
      gba_printf("There are %d bunnys.", 32);
      update_window = true;
   }
   if(keys & KEY_R){
      run_tests();
      update_window = true;
   }

   if(update_window){
      render_menu(false);
   }
}
