#include <stdint.h>
#include <math.h>

#include "gpumath.h"

vgpu_t vgpu;

inline double sin_deg(double angle){
   return sin(angle * M_PI / 180.0);
}

inline double cos_deg(double angle){
   return cos(angle * M_PI / 180.0);
}

inline uint16_t get_rot_pixel(int32_t x, int32_t y){
   int32_t angle = vgpu.output.angle;
   float flt_x = x * cos_deg(angle) + y * sin_deg(angle);
   float flt_y = y * cos_deg(angle) + x * sin_deg(angle);
   
   return vgpu.input.data[(int32_t)flt_y * vgpu.input.w + (int32_t)flt_x];
}

inline uint16_t get_scale_pixel(int32_t x, int32_t y){
   //0.0,0.0 is top left corner, 1.0,1.0 is bottom right corner
   float flt_x = x;
   float flt_y = y;
   
   //convert to 0.0-1.0
   flt_x /= vgpu.output.w;
   flt_y /= vgpu.output.h;
   
   //convert 0.0-1.0 to new image size
   flt_x *= vgpu.input.w;
   flt_y *= vgpu.input.h;
   
   return vgpu.input.data[(int32_t)flt_y * vgpu.input.w + (int32_t)flt_x];
}

void rotate(){
   for(int32_t cnt_y = 0; cnt_y < vgpu.output.h; cnt_y++){
      for(int32_t cnt_x = 0; cnt_x < vgpu.output.w; cnt_x++){
         vgpu.output.data[cnt_y * vgpu.output.w + cnt_x] = get_rot_pixel(cnt_x, cnt_y);
      }
   }
}

void scale(){
   for(int32_t cnt_y = 0; cnt_y < vgpu.output.h; cnt_y++){
      for(int32_t cnt_x = 0; cnt_x < vgpu.output.w; cnt_x++){
         vgpu.output.data[cnt_y * vgpu.output.w + cnt_x] = get_scale_pixel(cnt_x, cnt_y);
      }
   }
}
