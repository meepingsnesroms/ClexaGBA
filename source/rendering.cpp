#include <gba_video.h>
#include <gba_input.h>

#include <stdlib.h>//for malloc

#include "uguishim.h"
#include "gametypes.h"

//use framebuffer for now
static uint16_t *const vram = ((uint16_t*)0x06000000);

uint16_t* background;//[SCREEN_WIDTH * SCREEN_HEIGHT];

void init_renderer(){
   background = (uint16_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t));
   if(background == NULL){
      bsod("Not enough memory for \"uint16_t background[240 * 160];\"");
   }
}

void conv_32bpp_to_16(uint16_t* output, uint32_t* data, uint32_t size){
   for(uint32_t cnt = 0; cnt < size; cnt++){
      uint8_t visible = (data[cnt] >> 24) & 0xFF;
      if(visible != 0){
         uint8_t r = (data[cnt] >> 16) & 0xFF;
         uint8_t g = (data[cnt] >> 8) & 0xFF;
         uint8_t b = data[cnt] & 0xFF;
         output[cnt] = RGB8(b,g,r) | 0x8000/*visible bit*/;
      }
      else{
         //if invisible no need to calculate what shade of invisible :)
         output[cnt] = 0x0000;
      }
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

void draw_background(){
   for(uint32_t cnt = 0; cnt < SCREEN_WIDTH * SCREEN_HEIGHT; cnt++){
      vram[cnt] = background[cnt];
   }
}

void restore_background(entity& ent){
   if(!ent.dirty.is_dirty)return;
   for(uint16_t yinc = 0; yinc < ent.sprite.h; yinc++){
      for(uint16_t xinc = 0; xinc < ent.sprite.w; xinc++){
         vram[ent.dirty.x + xinc + ((ent.dirty.y + yinc) * SCREEN_WIDTH)] = background[ent.dirty.x + xinc + ((ent.dirty.y + yinc) * SCREEN_WIDTH)];
      }
   }
   ent.dirty.is_dirty = false;
}

void draw_texture(uint16_t x, uint16_t y, texture& tex){
   if(tex.bitmap == NULL)return;
   for(uint16_t yinc = 0; yinc < tex.h; yinc++){
      for(uint16_t xinc = 0; xinc < tex.w; xinc++){
         //check "is visible" bit
         uint16_t color = tex.bitmap[xinc + (yinc * tex.w)];
         if(color & 0x8000){
            vram[x + xinc + ((y + yinc) * SCREEN_WIDTH)] = color & 0x7FFF;//ent.bitmap[xinc + (yinc * ent.w)];
         }
      }
   }
}

void draw_texture_background(uint16_t x, uint16_t y, texture& tex){
   if(tex.bitmap == NULL)return;
   for(uint16_t yinc = 0; yinc < tex.h; yinc++){
      for(uint16_t xinc = 0; xinc < tex.w; xinc++){
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
