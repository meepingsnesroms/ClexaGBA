#include <gba_input.h>
#include "mainmenu.h"
#include "gameengine.h"

bool in_game = false;

void run_frame(){
   scanKeys();
   
   if(in_game){
      run_frame_game();
   }
   else{
      draw_menu();
   }
}
