#include <gba_input.h>
#include "menu.h"
#include "gameengine.h"

bool in_game = false;
static bool in_game_last_frame = false;

void run_frame(){
   
   scanKeys();
   
   if(in_game){
      if(!in_game_last_frame){
         switch_to_game();
      }
      run_frame_game();
   }
   else{
      if(in_game_last_frame){
         switch_to_menu();
      }
      draw_menu();
   }
   //game code here :)
   
}
