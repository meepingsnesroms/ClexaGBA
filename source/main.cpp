#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>

#include "runloop.h"
#include "gameengine.h"
#include "uguishim.h"
#include "mainmenu.h"
#include "rendering.h"
#include "inventory.h"
#include "audio.h"

#include "sram.h"
#include "diag.h"

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------


	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqEnable(IRQ_VBLANK);
   
   // set framebuffer mode
   SetMode(MODE_3 | BG2_ON /* 240*160 16bit color */);
   
   init_ugui();
   init_menu();
   init_renderer();
   init_audio();
   init_inventory();
   init_game();

   //check if the save data chip(or HDD/sdcard on computer or phone) has started to rot
   if(is_data_corrupt()){
      gba_printf("Your save data is corrupt!\nTo use hold L and R when closing.\nTo close this message press B.\nThe save will be formatted if not used!");
      uint16_t keys = ~(REG_KEYINPUT);
      if((keys & (KEY_L | KEY_R)) == (KEY_L | KEY_R)){
         make_valid_checksum();
         gba_printf("Using corrupt save!\nPress B!");
      }
      else{
         format_save();
         gba_printf("Save formatted!\nPress B!");
      }
   }
   
   switch_to_menu();
   
   while (1) {
      run_frame();
      VBlankIntrWait();
   }
}


