#include <gba_input.h>
#include <gba_video.h>
#include <gba_systemcalls.h>//for VBlankIntrWait()

#include <stdarg.h>
#include <stdio.h>

#include "ugui/ugui.h"

#define LINES_AT_ONCE 5

char printf_buffer[100];

static void reset_console(){
   UG_GUI* g = UG_CurrentGUI();
   
   g->console.x_start = 4;
   g->console.y_start = 4;
   g->console.x_end = g->x_dim - g->console.x_start-1;
   g->console.y_end = g->y_dim - g->console.x_start-1;
   g->console.x_pos = g->console.x_end;
   g->console.y_pos = g->console.y_end;
}

void gba_printf(char* str, ...){
   va_list argptr;
   va_start(argptr, str);
   vsprintf(printf_buffer, str, argptr);
   va_end(argptr);
   
   UG_ConsoleSetArea(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
   UG_ConsoleSetForecolor(C_WHITE);
   UG_ConsoleSetBackcolor(C_LIGHT_STEEL_BLUE);
   
   UG_ConsolePutString(printf_buffer);
   
   while(1){
      scanKeys();
      uint16_t keys = keysDown();
      
      //close message
      if(keys & KEY_B){
         break;
      }

      VBlankIntrWait();
   }
   
   //turn off the console
   reset_console();
}

void run_tests(){
   UG_ConsoleSetArea(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
   UG_ConsoleSetForecolor(C_WHITE);
   UG_ConsoleSetBackcolor(C_LIGHT_STEEL_BLUE);
   
   UG_ConsolePutString("This is a diagnostics menu:\n");
   UG_ConsolePutString("If you dont know C++ press B now.\n");
   UG_ConsolePutString("If you want debug data press A.\n\n");
   
   while(1){
      scanKeys();
      uint16_t keys = keysDown();
      
      //leave tests
      if(keys & KEY_B){
         break;
      }
      
      //print next * lines
      if(keys & KEY_A){
         
         //todo: print debug data here
         
      }
      
      VBlankIntrWait();
   }
   
   //turn off the console
   reset_console();
}
