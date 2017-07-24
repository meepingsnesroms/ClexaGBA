#include "timer.h"

void timer_reset(timer* timer_to_clear){
   (*timer_to_clear).days    = 0;
   (*timer_to_clear).hours   = 0;
   (*timer_to_clear).minutes = 0;
   (*timer_to_clear).seconds = 0;
   (*timer_to_clear).frames  = 0;
}

void timer_tick(timer* timer_to_tick){
   //tick timer
   (*timer_to_tick).frames++;
   
   //frames overflow
   if((*timer_to_tick).frames >= 60){
      (*timer_to_tick).seconds++;
      (*timer_to_tick).frames -= 60;
      
      //seconds overflow
      if((*timer_to_tick).seconds >= 60){
         (*timer_to_tick).minutes++;
         (*timer_to_tick).seconds -= 60;
         
         //minutes overflow
         if((*timer_to_tick).minutes >= 60){
            (*timer_to_tick).hours++;
            (*timer_to_tick).minutes -= 60;
            
            //hours overflow
            if((*timer_to_tick).hours >= 24){
               (*timer_to_tick).days++;
               (*timer_to_tick).hours -= 24;
            }
         }
      }
   }
}
