#include <gba_video.h>
#include <gba_input.h>
#include <gba_systemcalls.h>//for VBlankIntrWait()

#include "itemmenu.h"
#include "rendering.h"
#include "uguishim.h"
#include "ugui/ugui.h"

//using 8x12 font for item names

#define USE_SCROLLING_LIST

#define ITEM_WINDOW_WIDTH  ((SCREEN_WIDTH / 2) - 20)//((SCREEN_WIDTH / 2) + 20)
#define ITEM_WINDOW_HEIGHT SCREEN_HEIGHT

#define ITEM_LIST_OFFSET_X 0
#define ITEM_LIST_OFFSET_Y 0

#define ITEM_WIDTH  (UG_WindowGetInnerWidth(&window))
#define ITEM_HEIGHT 16 //2 pixel margins on top and bottom

#define ITEM_CURSOR_COLOR C_PALE_GOLDEN_ROD
#define ITEM_TEXT_COLOR   C_WHITE

/* 
 list items shown at once, must be a constant,
 using a function like UG_WindowGetInnerHeight(&window)
 will try to determine the object array sizes using a window varible
 before the window is initialised giving improper size.(likely 0)
*/
#define MAX_LIST_SIZE (SCREEN_HEIGHT / ITEM_HEIGHT)

#define IMAGE_BOX_OFFSET_X 160
#define IMAGE_BOX_OFFSET_Y 75
#define IMAGE_BOX_WIDTH  60
#define IMAGE_BOX_HEIGHT 60

static void message_cb(UG_MESSAGE* msg_ptr){
   //do nothing
}

item* list_items(item* items, uint32_t total_items, bool exit_allowed){
   UG_WINDOW   window;
   UG_OBJECT   objects[MAX_LIST_SIZE];
   UG_TEXTBOX  text_entrys[MAX_LIST_SIZE];
   uint32_t    active_item = 0;
   
   UG_WindowCreate(&window, objects, MAX_LIST_SIZE, message_cb);
   
   UG_WindowSetXStart(&window, 0);
   UG_WindowSetYStart(&window, 0);
   UG_WindowSetXEnd(&window, ITEM_WINDOW_WIDTH  - 1);
   UG_WindowSetYEnd(&window, ITEM_WINDOW_HEIGHT - 1);
   UG_WindowSetStyle(&window, WND_STYLE_HIDE_TITLE | WND_STYLE_3D);
   
   UG_WindowSetForeColor(&window, C_WHITE);
   
   uint32_t current_y = ITEM_LIST_OFFSET_Y;
   for(uint32_t cnt = 0; cnt < MAX_LIST_SIZE; cnt++){
      UG_TextboxCreate(&window, &text_entrys[cnt] /*(UG_TEXTBOX*)*/, cnt /*id*/, ITEM_LIST_OFFSET_X, current_y, ITEM_LIST_OFFSET_X + ITEM_WIDTH - 1, current_y + ITEM_HEIGHT - 1);
      UG_TextboxSetFont(&window, cnt /*id*/, &FONT_8X12);
      UG_TextboxSetAlignment(&window, cnt /*id*/, ALIGN_CENTER);
      UG_TextboxSetText(&window, cnt /*id*/, items[cnt].name);
      UG_TextboxShow(&window, cnt /*id*/);
      current_y += ITEM_HEIGHT - 1;
   }
   
   UG_TextboxSetBackColor(&window, active_item /*id*/, ITEM_CURSOR_COLOR);
   
   UG_WindowShow(&window);

   //frame loop
   uint16_t window_background_color = UG_WindowGetBackColor(&window);
   uint32_t old_active_item;
   uint32_t shifted_by = 0;
   uint32_t old_shifted_by;
   bool needs_render = true;
   while(1){
      old_active_item = active_item;
      old_shifted_by  = shifted_by;
      
      //test buttons
      scanKeys();
      uint16_t keys = keysDown();
      
      if(keys & KEY_A){
         break;//an item was selected
      }
      
      if(exit_allowed && (keys & KEY_B)){
         return (item*)NULL;//exit wanted, no item was selected
      }
      
      //top of the list is 0, so going up means subtracting not adding
      if(keys & KEY_UP){
         if(active_item > 0)active_item--;
         else{
#ifdef USE_SCROLLING_LIST
            if(shifted_by > 0){
               shifted_by--;
               //need to fetch next item from list and push list down
               for(uint32_t cnt = 0; cnt < MAX_LIST_SIZE; cnt++){
                  UG_TextboxSetText(&window, cnt /*id*/, items[cnt + shifted_by].name);
               }
            }
#endif
         }
      }
      
      if(keys & KEY_DOWN){
         if(active_item < MAX_LIST_SIZE - 1 && active_item < total_items - 1)active_item++;
         else{
#ifdef USE_SCROLLING_LIST
            //check if scrolling is ok
            if(active_item + shifted_by < total_items - 1){
               shifted_by++;
               //need to fetch next item from list and push list up
               for(uint32_t cnt = 0; cnt < MAX_LIST_SIZE; cnt++){
                  UG_TextboxSetText(&window, cnt /*id*/, items[cnt + shifted_by].name);
               }
            }
#endif
         }
      }

      //update gui
      if(old_active_item != active_item || shifted_by != old_shifted_by){
         needs_render = true;
      }
      
      if(needs_render){
         UG_TextboxSetBackColor(&window, old_active_item /*id*/, window_background_color);
         UG_TextboxSetBackColor(&window, active_item /*id*/, ITEM_CURSOR_COLOR);
         
         UG_Update();
         
         //draw the item
         Fake_Window(IMAGE_BOX_OFFSET_X, IMAGE_BOX_OFFSET_Y, IMAGE_BOX_OFFSET_X + IMAGE_BOX_WIDTH, IMAGE_BOX_OFFSET_Y + IMAGE_BOX_HEIGHT);
         if(items[active_item + shifted_by].item_image.bitmap != NULL){
            draw_texture(IMAGE_BOX_OFFSET_X, IMAGE_BOX_OFFSET_Y, items[active_item + shifted_by].item_image);
         }
         
         needs_render = false;
      }
      
      //wait for next frame
      VBlankIntrWait();
   }
   
   //destroy the window to prevent use after free on return from function
   UG_WindowDelete(&window);
   
   return &items[active_item + shifted_by];//return selected item
}
