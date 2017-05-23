#include <gba_input.h>
#include <gba_video.h>
#include <gba_systemcalls.h>//for VBlankIntrWait()

#include "ugui/ugui.h"

#define LINES_AT_ONCE 5

void run_tests(){
   UG_ConsoleSetArea(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
   UG_ConsoleSetForecolor(C_WHITE);
   UG_ConsoleSetBackcolor(C_LIGHT_STEEL_BLUE);
   
   UG_ConsolePutString("This is a diagnostics menu\n");
   UG_ConsolePutString("If you dont know C++ or are just trying to play the game press B now.");
   
   while(1){
      uint16_t keys;
      scanKeys();
      keys = keysDown();
      
      //leave tests
      if(keys & KEY_B){
         break;
      }
      
      //print next * lines
      if(keys & KEY_A){
         break;
      }
      
      
      
      
      
      VBlankIntrWait();
   }
   
   //turn off the console
   UG_ConsoleSetArea(0, 0, 0, 0);
}
