#include <gba_input.h>
#include <gba_video.h>
#include <gba_systemcalls.h>//for VBlankIntrWait()

#include <stdarg.h>
#include <stdio.h>

#include "ugui/ugui.h"
#include "diag.h"



//test functions
//headers required by the tests
#include "gametypes.h"
#include "speedhacks.h"
#include "rendering.h"
#include "gpumath.h"
#include "assets.h"

bool rotate_test(){
   uint16_t source[16 * 16];
   uint16_t dest[16 * 16];
   
   memcpy16(source, clarke_front_data, 16 * 16);
   
   vgpu.input.w = 16;
   vgpu.input.h = 16;
   vgpu.input.data = source;
   
   vgpu.output.w = 16;
   vgpu.output.h = 16;
   //vgpu.output.angle = 90;//can be anything from 0 to 359
   vgpu.output.angle = 10;//can be anything from 0 to 359
   vgpu.output.data = dest;
   
   while(1){
      scanKeys();
      uint16_t keys_buffered = keysDown();
      if(keys_buffered & KEY_A){
         break;
      }
      
      uint16_t keys = ~(REG_KEYINPUT);
      
      if(keys & KEY_LEFT){
         if(vgpu.output.angle == 0)vgpu.output.angle = 359;
         else vgpu.output.angle--;
      }
      else if(keys & KEY_RIGHT){
         if(vgpu.output.angle == 359)vgpu.output.angle = 0;
         else vgpu.output.angle++;
      }
      
      rotate();
      
      texture output = {16, 16, dest};
      draw_texture(0, 0, output);
      
      VBlankIntrWait();
   }
   
   return false;//no text to print
}

bool scale_test(){
   uint16_t source[16 * 16];
   uint16_t dest[65 * 65];
   
   memcpy16(source, clarke_front_data, 16 * 16);
   
   vgpu.input.w = 16;
   vgpu.input.h = 16;
   vgpu.input.data = source;
   
   vgpu.output.w = 65;
   vgpu.output.h = 65;
   vgpu.output.data = dest;
   
   scale();
   
   texture output = {65, 65, dest};
   
   draw_texture(0, 0, output);
   
   wait_for_key_press(KEY_A);
   
   return false;//no text to print
}

bool scale_rot_test(){
   uint16_t source[65 * 65];
   uint16_t dest[65 * 65];
   
   //scale
   vgpu.input.w = 16;
   vgpu.input.h = 16;
   vgpu.input.data = clarke_front_data;
   
   vgpu.output.w = 65;
   vgpu.output.h = 65;
   vgpu.output.data = source;
   
   scale();
   
   //rotate
   vgpu.input.w = 65;
   vgpu.input.h = 65;
   vgpu.input.data = source;
   
   vgpu.output.w = 65;
   vgpu.output.h = 65;
   vgpu.output.angle = 37;//can be anything from 0 to 359
   vgpu.output.data = dest;
   
   while(1){
      scanKeys();
      uint16_t keys_buffered = keysDown();
      if(keys_buffered & KEY_A){
         break;
      }
      
      uint16_t keys = ~(REG_KEYINPUT);
      
      if(keys & KEY_LEFT){
         if(vgpu.output.angle == 0)vgpu.output.angle = 359;
         else vgpu.output.angle--;
      }
      else if(keys & KEY_RIGHT){
         if(vgpu.output.angle == 359)vgpu.output.angle = 0;
         else vgpu.output.angle++;
      }
      
      rotate();
      
      texture output = {65, 65, dest};
      draw_texture(0, 0, output);
      
      VBlankIntrWait();
   }
   
   return false;//no text to print
}

//end of test functions



diag_test tests[] = {{rotate_test, "Rotation Test\n"}, {scale_test, "Scaling Test\n"}, {scale_rot_test, "Scale And Rotate Test\n"}, {NULL, "butter flea"}};
char test_result[200];//if a diag_test function returns true this will be printed

static void reset_console(){
   UG_GUI* g = UG_CurrentGUI();
   
   g->console.x_start = 4;
   g->console.y_start = 4;
   g->console.x_end = g->x_dim - g->console.x_start-1;
   g->console.y_end = g->y_dim - g->console.x_start-1;
   g->console.x_pos = g->console.x_end;
   g->console.y_pos = g->console.y_end;
}

void wait_for_key_press(uint16_t key){
   while(1){
      scanKeys();
      uint16_t keys = keysDown();
      
      if(keys & key){
         break;
      }
      
      VBlankIntrWait();
   }
}

void gba_printf(char* str, ...){
   static char printf_buffer[200];
   va_list argptr;
   va_start(argptr, str);
   vsprintf(printf_buffer, str, argptr);
   va_end(argptr);
   
   UG_ConsoleSetArea(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
   UG_ConsoleSetForecolor(C_WHITE);
   UG_ConsoleSetBackcolor(C_LIGHT_STEEL_BLUE);
   
   UG_ConsolePutString(printf_buffer);
   
   wait_for_key_press(KEY_B);
   
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
   
   
   uint32_t old_test_num = (uint32_t)-1;
   uint32_t test_num = 0;
   while(1){
      if(test_num != old_test_num){
         if(tests[test_num].test_func == NULL){
            //no more tests
            break;
         }
         else{
            UG_ConsolePutString(tests[test_num].name);//print next test name
            old_test_num = test_num;
         }
      }
      
      scanKeys();
      uint16_t keys = keysDown();
      
      //leave tests
      if(keys & KEY_B){
         break;
      }
      
      //print next * lines
      if(keys & KEY_A){
         bool print_test_result = tests[test_num].test_func();
         if(print_test_result){
            UG_ConsolePutString(test_result);
         }
         
         test_num++;
      }
      
      VBlankIntrWait();
   }
   
   //turn off the console
   reset_console();
}
