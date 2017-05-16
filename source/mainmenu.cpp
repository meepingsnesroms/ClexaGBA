#include <gba_video.h>//for SCREEN_* macros
#include <gba_input.h>

#include "uguishim.h"
#include "runloop.h"
#include "gameengine.h"
#include "itemmenu.h"

#define MAX_OBJECTS 50
#define NUM_BUTTONS 2

UG_WINDOW   render_window;
UG_OBJECT   render_objects[MAX_OBJECTS];
UG_BUTTON   render_buttons[NUM_BUTTONS];
uint8_t     render_selected_button;

static void message_cb(UG_MESSAGE* msg_ptr){
   //do nothing
}

void init_menu(){
   UG_WindowCreate(&render_window, render_objects, MAX_OBJECTS, message_cb);
   
   UG_WindowSetXStart(&render_window, 0);
   UG_WindowSetYStart(&render_window, 0);
   UG_WindowSetXEnd(&render_window, SCREEN_WIDTH);
   UG_WindowSetYEnd(&render_window, SCREEN_HEIGHT);
   UG_WindowSetTitleColor(&render_window, C_BLUE);
   UG_WindowSetTitleTextColor(&render_window, C_WHITE);
   UG_WindowSetTitleTextAlignment(&render_window, ALIGN_CENTER);
   UG_WindowSetTitleText(&render_window, "BIOS 2117");
   
   //Start button
   UG_ButtonCreate(&render_window, &render_buttons[0], 0 /*id*/, 0 /*xs*/, 0 /*ys*/, 80 /*xe*/, 20 /*ye*/);
   UG_ButtonSetText(&render_window, 0 /*id*/, "Start");
   UG_ButtonShow(&render_window, 0);
   render_selected_button = 0;
   UG_ButtonSetStyle(&render_window, 0, BTN_STYLE_2D);
   
   //Options button
   UG_ButtonCreate(&render_window, &render_buttons[1], 1 /*id*/, UG_WindowGetInnerWidth(&render_window) - 1 - 80 /*xs*/, 0 /*ys*/, UG_WindowGetInnerWidth(&render_window) - 1 /*xe*/, 20 /*ye*/);
   UG_ButtonSetText(&render_window, 1 /*id*/, "Options");
   UG_ButtonShow(&render_window, 1);
   
   
   UG_WindowShow(&render_window);
   UG_Update();
}

void switch_to_menu(){
   //SetMode(MODE_3 | BG2_ON /* 240*160 16bit color */);
}

void draw_menu(){
   bool update_window = false;
   uint16_t keys = keysDown();
   
   if(keys & KEY_LEFT){
      if(render_selected_button > 0){
         UG_ButtonSetStyle(&render_window, render_selected_button, BTN_STYLE_3D);
         render_selected_button--;
         UG_ButtonSetStyle(&render_window, render_selected_button, BTN_STYLE_2D);
         update_window = true;
      }
   }
   
   if(keys & KEY_RIGHT){
      if(render_selected_button < NUM_BUTTONS - 1){
         UG_ButtonSetStyle(&render_window, render_selected_button, BTN_STYLE_3D);
         render_selected_button++;
         UG_ButtonSetStyle(&render_window, render_selected_button, BTN_STYLE_2D);
         update_window = true;
      }
   }
   
   //select button
   if(keys & KEY_A){
      switch(render_selected_button){
         case 0:
            //Start button
            UG_WindowHide(&render_window);
            in_game = true;
            update_window = false;
            switch_to_game();
            break;
         case 1:
            //Options button
            //list_items((item*)NULL);//test, will be removed
            break;
         default:
            //do nothing
            break;
      }
   }

   if(update_window){
      UG_Update();
   }
}
