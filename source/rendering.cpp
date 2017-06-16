#include <gba_video.h>
#include <gba_input.h>

#include <stdlib.h>//for malloc

#include "uguishim.h"
#include "gametypes.h"
#include "speedhacks.h"

uint16_t* vram = ((uint16_t*)VRAM);
uint16_t* background;//[SCREEN_WIDTH * SCREEN_HEIGHT];

void init_renderer(){
   background = (uint16_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));
   if(background == NULL){
      bsod("Not enough memory for \"uint16_t background[240 * 160];\"");
   }
}

void invert_color(uint16_t* data, uint32_t size){
   for(uint32_t cnt = 0; cnt < size; cnt++){
      //only modify visible pixels
      if(data[cnt] & 0x8000){
         data[cnt] = (~data[cnt]) | 0x8000;
      }
   }
}

uint16_t blend_color(uint16_t arg1, uint16_t arg2){
   uint8_t r[3];
   uint8_t g[3];
   uint8_t b[3];
   
   //this function ignores transparency
   arg1 &= 0x7FFF;
   arg2 &= 0x7FFF;
   
   r[0] = arg1 & 0x001F;
   g[0] = (arg1 >> 5) & 0x001F;
   b[0] = (arg1 >> 10) & 0x001F;
   
   r[1] = arg2 & 0x001F;
   g[1] = (arg2 >> 5) & 0x001F;
   b[1] = (arg2 >> 10) & 0x001F;
   
   r[2] = (r[0] + r[1]) >> 1;
   g[2] = (g[0] + g[1]) >> 1;
   b[2] = (b[0] + b[1]) >> 1;
   
   return RGB5(b[2],g[2],r[2]);
}

void draw_background(){
   memcpy32(vram, background, SCREEN_WIDTH * SCREEN_HEIGHT / 2);
}

void fill_background(uint16_t color){
   uint32_t fill_val = ((color << 16) | color) & 0x7FFF7FFF;//remove transparent bit
   memset32(background, fill_val, SCREEN_WIDTH * SCREEN_HEIGHT / 2);
}

void restore_background(int32_t x, int32_t y, int32_t w, int32_t h){
   for(int32_t yinc = 0; yinc < h; yinc++){
      memcpy16(&vram[x + ((y + yinc) * SCREEN_WIDTH)], &background[x + ((y + yinc) * SCREEN_WIDTH)], w);
   }
}

void restore_background(entity& ent){
   if(!ent.dirty.is_dirty)return;
   
   int32_t clip_x = ent.dirty.x;
   int32_t clip_y = ent.dirty.y;
   int32_t width  = ent.sprite.w;
   int32_t height = ent.sprite.h;
   
   while(clip_x < 0){
      clip_x++;
      width--;
   }
   
   while(clip_y < 0){
      clip_y++;
      height--;
   }
   
   while(clip_x + width > SCREEN_WIDTH){
      width--;
   }
   
   while(clip_y + height > SCREEN_HEIGHT){
      height--;
   }
   
   if(width > 0 && height > 0)restore_background(clip_x, clip_y, width, height);
   ent.dirty.is_dirty = false;
}

void draw_texture(int32_t x, int32_t y, texture& tex){
   if(tex.bitmap == NULL)return;

   if(x < 0 || y < 0 || x + tex.w >= SCREEN_WIDTH || y + tex.h >= SCREEN_HEIGHT){
      //if offscreen rendering, use this safe function
      for(int32_t yinc = 0; yinc < tex.h; yinc++){
         for(int32_t xinc = 0; xinc < tex.w; xinc++){
            //check "is visible" bit
            uint16_t color = tex.bitmap[xinc + (yinc * tex.w)];
            if(color & 0x8000 && x + xinc > -1 && y + yinc > -1 && x + xinc < SCREEN_WIDTH && y + yinc < SCREEN_HEIGHT){
               vram[x + xinc + ((y + yinc) * SCREEN_WIDTH)] = color & 0x7FFF;//ent.bitmap[xinc + (yinc * ent.w)];
            }
         }
      }
   }
   else{
      //not on an edge, use this fast function
      for(int32_t yinc = 0; yinc < tex.h; yinc++){
         for(int32_t xinc = 0; xinc < tex.w; xinc++){
            //check "is visible" bit
            uint16_t color = tex.bitmap[xinc + (yinc * tex.w)];
            if(color & 0x8000){
               vram[x + xinc + ((y + yinc) * SCREEN_WIDTH)] = color & 0x7FFF;//ent.bitmap[xinc + (yinc * ent.w)];
            }
         }
      }
   }
}

void draw_texture_from_midpoint(int32_t x, int32_t y, texture& tex){
   draw_texture(x - (tex.w / 2), y - (tex.h / 2), tex);
}

void draw_texture_background(int32_t x, int32_t y, texture& tex){
   if(tex.bitmap == NULL)return;
   for(int32_t yinc = 0; yinc < tex.h; yinc++){
      for(int32_t xinc = 0; xinc < tex.w; xinc++){
         //check "is visible" bit
         uint16_t color = tex.bitmap[xinc + (yinc * tex.w)];
         if(color & 0x8000){
            background[x + xinc + ((y + yinc) * SCREEN_WIDTH)] = color & 0x7FFF;//ent.bitmap[xinc + (yinc * ent.w)];
         }
      }
   }
}

void draw_entity(entity& ent){
   draw_texture(ent.x + ent.sprite_x_offset, ent.y + ent.sprite_y_offset, ent.sprite);
   ent.dirty.x = ent.x + ent.sprite_x_offset;
   ent.dirty.y = ent.y + ent.sprite_y_offset;
   ent.dirty.is_dirty = true;
}

void draw_entity_background(entity& ent){
   draw_texture_background(ent.x + ent.sprite_x_offset, ent.y + ent.sprite_y_offset, ent.sprite);
}
