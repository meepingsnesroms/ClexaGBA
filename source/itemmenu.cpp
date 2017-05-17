#include <gba_video.h>
#include <gba_input.h>
#include <gba_interrupt.h>//for VBlankIntrWait()
#include <gba_systemcalls.h>//for VBlankIntrWait()

#include "itemmenu.h"
#include "uguishim.h"
#include "ugui/ugui.h"

#define MAX_ITEMS 50

//using 8x12 font for item names

#define ITEM_WINDOW_WIDTH  ((SCREEN_WIDTH / 2) - 20)
#define ITEM_WINDOW_HEIGHT SCREEN_HEIGHT

#define ITEM_LIST_OFFSET_X 20
#define ITEM_LIST_OFFSET_Y 20
#define ITEM_WIDTH  60
#define ITEM_HEIGHT 16 //2 pixel margins on top and bottom

#define ITEM_CURSOR_COLOR C_PALE_GOLDEN_ROD
#define ITEM_TEXT_COLOR   C_WHITE
#define MAX_LIST_SIZE (SCREEN_HEIGHT / ITEM_HEIGHT) //list items shown at once

#define IMAGE_BOX_OFFSET_X 160
#define IMAGE_BOX_OFFSET_Y 75
#define IMAGE_BOX_WIDTH  50
#define IMAGE_BOX_HEIGHT 50

static void message_cb(UG_MESSAGE* msg_ptr){
   //do nothing
}

item* list_items(item* items){   
   UG_WINDOW   window;
   UG_OBJECT   objects[MAX_ITEMS];
   UG_TEXTBOX  text_entrys[MAX_LIST_SIZE];
   uint8_t     active_item = 0;
   
   UG_WindowCreate(&window, objects, MAX_ITEMS, message_cb);
   
   UG_WindowSetXStart(&window, 0);
   UG_WindowSetYStart(&window, 0);
   UG_WindowSetXEnd(&window, ITEM_WINDOW_WIDTH);
   UG_WindowSetYEnd(&window, ITEM_WINDOW_HEIGHT);
   UG_WindowSetStyle(&window, WND_STYLE_HIDE_TITLE | WND_STYLE_3D);
   
   UG_WindowSetForeColor(&window, C_WHITE);
   
   uint16_t current_x = ITEM_LIST_OFFSET_X;
   uint16_t current_y = ITEM_LIST_OFFSET_Y;
   for(uint8_t cnt = 0; cnt < MAX_LIST_SIZE; cnt++){
      UG_TextboxCreate(&window, &text_entrys[cnt] /*(UG_TEXTBOX*)*/, cnt /*id*/, current_x, current_y, current_x + ITEM_WIDTH, current_y + ITEM_HEIGHT);
      UG_TextboxSetFont(&window, cnt /*id*/, &FONT_8X12);
      UG_TextboxSetAlignment(&window, cnt /*id*/, ALIGN_CENTER_LEFT);
      UG_TextboxSetText(&window, cnt /*id*/, items[cnt].name);
      //UG_TextboxSetForeColor(&window, cnt /*id*/, ITEM_TEXT_COLOR);
      UG_TextboxShow(&window, cnt /*id*/);
      current_y += ITEM_HEIGHT;
   }
   
   UG_TextboxSetBackColor(&window, active_item /*id*/, ITEM_CURSOR_COLOR);
   
   UG_WindowShow(&window);
   
   //only render this once
   Fake_Window(IMAGE_BOX_OFFSET_X, IMAGE_BOX_OFFSET_Y, IMAGE_BOX_OFFSET_X + IMAGE_BOX_WIDTH, IMAGE_BOX_OFFSET_Y + IMAGE_BOX_HEIGHT);

   //frame loop
   uint16_t window_background_color = UG_WindowGetBackColor(&window);
   uint8_t old_active_item;
   bool needs_render = true;
   while(1){
      old_active_item = active_item;
      
      //test buttons
      scanKeys();
      uint16_t keys = keysDown();
      
      if(keys & KEY_A){
         break;//an item was selected
      }
      
      //top of the list is 0, so going up means subtracting not adding
      if(keys & KEY_UP){
         if(active_item > 0)active_item--;
      }
      
      if(keys & KEY_DOWN){
         if(active_item < MAX_LIST_SIZE)active_item++;
      }
      
      //update gui
      if(old_active_item != active_item){
         UG_TextboxSetBackColor(&window, old_active_item /*id*/, window_background_color);
         UG_TextboxSetBackColor(&window, active_item /*id*/, ITEM_CURSOR_COLOR);
         needs_render = true;
      }
      
      if(needs_render){
         UG_Update();
         
         //draw the item
         
         needs_render = false;
      }
      
      //wait for next frame
      VBlankIntrWait();
   }
   
   //destroy the window to prevent use after free on return from function
   UG_WindowDelete(&window);
   
   return &items[active_item];//return selected item
}
