#include <gba_video.h>

#include "ugui/ugui.h"

#define MAX_OBJECTS 50

//using 240*160 16bit color, mode 3
uint16_t*   vram = ((uint16_t*)0x06000000);
UG_GUI      renderer;
UG_WINDOW   render_window;
UG_OBJECT   render_objects[MAX_OBJECTS];
uint16_t    render_current_object;

static void gba_plot_pixel(int16_t x, int16_t y, uint16_t color){
   uint16_t fixed = 0;//gba has red and blue swapped
   fixed |= color >> 11;//red
   fixed |= color << 10;//blue
   fixed |= (color & 0x07C0) >> 1;//green, use top 5 of 6 green bits
   vram[x + (y * 240)] = fixed;
}

static void message_cb(UG_MESSAGE* msg_ptr){
   //do nothing
}

void init_menu(){
   UG_Init(&renderer, gba_plot_pixel, 240, 160);
   UG_WindowCreate(&render_window, render_objects, MAX_OBJECTS, message_cb);
   render_current_object = 0;
   
   
   UG_FontSelect(&FONT_6X8);
   //UG_SetForecolor(C_WHITE);
   //UG_SetBackcolor(C_BLUE);
   
   //UG_FillScreen(C_BLUE);
   
   UG_WindowSetXStart(&render_window, 0);
   UG_WindowSetYStart(&render_window, 0);
   UG_WindowSetXEnd(&render_window, 240);
   UG_WindowSetYEnd(&render_window, 160);
   UG_WindowSetTitleColor(&render_window, C_BLUE);
   UG_WindowSetTitleTextColor(&render_window, C_WHITE);
   UG_WindowSetTitleTextAlignment(&render_window, ALIGN_CENTER);
   UG_WindowSetTitleText(&render_window, "Placeholder");
   
   UG_WindowShow(&render_window);
}

void switch_to_menu(){
   SetMode(MODE_3 | BG2_ON /* 240*160 16bit color */);
   
   UG_WindowShow(&render_window);
}

void draw_menu(){
   
   //UG_PutString(0, 0, "Fleas");
   //UG_WindowShow(&render_window);
   UG_Update();
   
}
