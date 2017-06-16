#include <stdint.h>

typedef struct{
   struct{
      int16_t w;
      int16_t h;
      uint16_t* data;
   }input;
   struct{
      int16_t w;
      int16_t h;
      uint16_t angle;
      uint16_t* data;
   }output;
}vgpu_t;

extern vgpu_t vgpu;

void rotate();
void scale();
