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
   
   // setup ugui
   init_ugui();
   
   // setup menu structs
   init_menu();
   
   // setup game structs
   init_game();

   while (1) {
      run_frame();
      VBlankIntrWait();
   }
}


