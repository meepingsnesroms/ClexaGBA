void draw_square(uint16_t x, uint16_t y, uint16_t color){
   for(uint16_t yinc = 0; yinc < 16; yinc++){
      for(uint16_t xinc = 0; xinc < 16; xinc++){
         vram[x + xinc + ((y + yinc) * SCREEN_WIDTH)] = color;
      }
   }
}

void restore_square(uint16_t x, uint16_t y){
   for(uint16_t yinc = 0; yinc < 16; yinc++){
      for(uint16_t xinc = 0; xinc < 16; xinc++){
         vram[x + xinc + ((y + yinc) * SCREEN_WIDTH)] = background[x + xinc + ((y + yinc) * SCREEN_WIDTH)];
      }
   }
}
