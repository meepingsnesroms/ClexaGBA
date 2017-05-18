#pragma once

#include <stdint.h>

typedef struct{
   uint16_t days;
   uint8_t hours;
   uint8_t minutes;
   uint8_t seconds;
   uint8_t frames;
}timer;

void timer_tick(timer& timer_to_tick);
