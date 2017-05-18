#include <gba_video.h>

#include <string.h>//for memcpy

#include "ugui/ugui.h"
#include "runloop.h"
#include "gameengine.h"
#include "uguishim.h"
#include "speedhacks.h"

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
   //gba has red and blue swapped, so it is actually bgr16
   uint16_t fixed = color >> 11;    //red
   fixed |= color << 10;            //blue
   fixed |= (color & 0x07C0) >> 1;  //green, use top 5 of 6 green bits
   vram[x + (y * SCREEN_WIDTH)] = fixed;
}

static UG_RESULT gba_fill_square(int16_t x, int16_t y, int16_t x2, int16_t y2, uint16_t color){
   uint16_t fixed = 0;//gba has red and blue swapped
   fixed |= color >> 11;//red
   fixed |= color << 10;//blue
   fixed |= (color & 0x07C0) >> 1;//green, use top 5 of 6 green bits

   for(int16_t cnt_y = y; cnt_y <= y2; cnt_y++){
      memset16(&vram[x + (cnt_y * SCREEN_WIDTH)], fixed, x2 - x + 1);
   }
   
   return UG_RESULT_OK;
}

static UG_RESULT gba_draw_line(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c){
   //fix color
   uint16_t fixed = 0;//gba has red and blue swapped
   fixed |= c >> 11;//red
   fixed |= c << 10;//blue
   fixed |= (c & 0x07C0) >> 1;//green, use top 5 of 6 green bits
   
   if(y1 == y2){
      memset16(&vram[x1 + (y1 * SCREEN_WIDTH)], fixed, x2 - x1 + 1);
      return UG_RESULT_OK;
   }
   
   if(x1 == x2){
      while(y1 <= y2){
         vram[x1 + (y1 * SCREEN_WIDTH)] = fixed;
         y1++;
      }
      return UG_RESULT_OK;
   }
   
   UG_S16 n, dx, dy, sgndx, sgndy, dxabs, dyabs, x, y, drawx, drawy;
   
   dx = x2 - x1;
   dy = y2 - y1;
   dxabs = (dx>0)?dx:-dx;
   dyabs = (dy>0)?dy:-dy;
   sgndx = (dx>0)?1:-1;
   sgndy = (dy>0)?1:-1;
   x = dyabs >> 1;
   y = dxabs >> 1;
   drawx = x1;
   drawy = y1;
   
   vram[drawx + (drawy * SCREEN_WIDTH)] = fixed;
   
   if( dxabs >= dyabs )
   {
      for( n=0; n<dxabs; n++ )
      {
         y += dyabs;
         if( y >= dxabs )
         {
            y -= dxabs;
            drawy += sgndy;
         }
         drawx += sgndx;
         vram[drawx + (drawy * SCREEN_WIDTH)] = fixed;
      }
   }
   else
   {
      for( n=0; n<dyabs; n++ )
      {
         x += dxabs;
         if( x >= dyabs )
         {
            x -= dyabs;
            drawx += sgndx;
         }
         drawy += sgndy;
         vram[drawx + (drawy * SCREEN_WIDTH)] = fixed;
      }
   }  

   return UG_RESULT_OK;
}

void init_ugui(){
   UG_Init(&screen_context, gba_plot_pixel, SCREEN_WIDTH, SCREEN_HEIGHT);
   
   UG_DriverRegister(DRIVER_DRAW_LINE, (void*)gba_draw_line);
   UG_DriverRegister(DRIVER_FILL_FRAME, (void*)gba_fill_square);
   //UG_DriverRegister(DRIVER_FILL_AREA, void* driver );
   
   UG_DriverEnable(DRIVER_DRAW_LINE);
   UG_DriverEnable(DRIVER_FILL_FRAME);
   //UG_DriverEnable(DRIVER_FILL_AREA);
   
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
