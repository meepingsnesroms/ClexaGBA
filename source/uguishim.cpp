#include <gba_video.h>

#include <string.h>//for memcpy

#include "ugui/ugui.h"
#include "runloop.h"
#include "gameengine.h"
#include "uguishim.h"

//using 240*160 16bit color, mode 3
static uint16_t *const vram = ((uint16_t*)0x06000000);
UG_GUI screen_context;

//used to make fake windows
UG_COLOR Frame_Colors[] =
{
   0x632C,
   0x632C,
   0x632C,
   0x632C,
   
   0xFFFF,
   0xFFFF,
   0x6B4D,
   0x6B4D,
   
   0xE71C,
   0xE71C,
   0x9D13,
   0x9D13,
};

static void gba_plot_pixel(int16_t x, int16_t y, uint16_t color){
   uint16_t fixed = 0;//gba has red and blue swapped
   fixed |= color >> 11;//red
   fixed |= color << 10;//blue
   fixed |= (color & 0x07C0) >> 1;//green, use top 5 of 6 green bits
   vram[x + (y * SCREEN_WIDTH)] = fixed;
}

static void gba_fill_square(int16_t x, int16_t y, int16_t x2, int16_t y2, uint16_t color){
   uint16_t fixed = 0;//gba has red and blue swapped
   fixed |= color >> 11;//red
   fixed |= color << 10;//blue
   fixed |= (color & 0x07C0) >> 1;//green, use top 5 of 6 green bits

   for(int16_t cnt_y = y; cnt_y < y2; cnt_y++){
      for(int16_t cnt_x = x; cnt_x < x2; cnt_x++){
         vram[cnt_x + (cnt_y * SCREEN_WIDTH)] = fixed;
      }
   }
}

void init_ugui(){
   UG_Init(&screen_context, gba_plot_pixel, SCREEN_WIDTH, SCREEN_HEIGHT);
   
   //UG_DriverRegister( UG_U8 type, void* driver );
   UG_DriverRegister(DRIVER_FILL_FRAME, (void*)gba_fill_square);
   //UG_DriverRegister( UG_U8 type, void* driver );
   
   //UG_DriverEnable( UG_U8 type );
   UG_DriverEnable(DRIVER_FILL_FRAME);
   //UG_DriverEnable( UG_U8 type );
   
   UG_FontSelect(&FONT_6X8);
}

void bsod(char* text){
   UG_SetForecolor(C_WHITE);
   UG_SetBackcolor(C_BLUE);
   UG_FillScreen(C_BLUE);
   UG_PutString(0, 0, text);
   while(1);//terminate excution
}

void Fake_Window_Frame(UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye, UG_COLOR* p){
   // Frame 0
   UG_DrawLine(xs, ys  , xe-1, ys  , *p++);
   UG_DrawLine(xs, ys+1, xs  , ye-1, *p++);
   UG_DrawLine(xs, ye  , xe  , ye  , *p++);
   UG_DrawLine(xe, ys  , xe  , ye-1, *p++);
   // Frame 1
   UG_DrawLine(xs+1, ys+1, xe-2, ys+1, *p++);
   UG_DrawLine(xs+1, ys+2, xs+1, ye-2, *p++);
   UG_DrawLine(xs+1, ye-1, xe-1, ye-1, *p++);
   UG_DrawLine(xe-1, ys+1, xe-1, ye-2, *p++);
   // Frame 2
   UG_DrawLine(xs+2, ys+2, xe-3, ys+2, *p++);
   UG_DrawLine(xs+2, ys+3, xs+2, ye-3, *p++);
   UG_DrawLine(xs+2, ye-2, xe-2, ye-2, *p++);
   UG_DrawLine(xe-2, ys+2, xe-2, ye-3, *p);
}

void Fake_Window(UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye){
   UG_FillFrame(xs + 1, ys + 1, xe - 1, ye - 1, C_WINDOW);
   Fake_Window_Frame(xs, ys, xe, ye, Frame_Colors);
}
