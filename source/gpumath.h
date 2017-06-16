#pragma once

#include <stdint.h>

typedef struct{
   struct{
      int32_t w;
      int32_t h;
      uint16_t* data;
   }input;
   struct{
      int32_t w;
      int32_t h;
      int32_t angle;
      uint16_t* data;
   }output;
}vgpu_t;

extern vgpu_t vgpu;

void rotate();
void scale();
