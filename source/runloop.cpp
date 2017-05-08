#include <gba_input.h>
#include "menu.h"
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
