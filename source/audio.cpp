#include <gba_sound.h>
#include <gba_timers.h>
#include <gba_interrupt.h>
#include <gba_dma.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "assets.h"
#include "uguishim.h"
#include "diag.h"
#include "speedhacks.h"

//#include "../data/smallmiku.cdata"
#include "../data/smallmiku_clean.cdata"

//this file is based on the example from http://deku.gbadev.org/program/sound2.html

#define SOUND_ENABLE    SNDSTAT_ENABLE
#define TIMER_CASCADE   TIMER_COUNT
#define DMA_MODE_FIFO   DMA_SPECIAL

int8_t*  pcm_data;
uint32_t pcm_offset;
uint32_t pcm_size;
volatile uint8_t  active_buffer;
volatile uint32_t audio_buffer[2][88];//352 bytes, 21024hz
volatile bool     loop_audio;

inline uint32_t swap32(uint32_t ui){
   ui = (ui >> 24) |
   ((ui<<8) & 0x00FF0000) |
   ((ui>>8) & 0x0000FF00) |
   (ui << 24);
   return ui;
}

static void swap_buffer(){
   uint8_t old_buffer = active_buffer;
   
   active_buffer++;
   active_buffer &= 1;
   
   //kill the audio to prevent an overflow
   REG_DMA1CNT = 0;
   
   //switch buffer
   REG_DMA1SAD = (uint32_t) &audio_buffer[active_buffer][0];
   REG_DMA1CNT = DMA_REPEAT | DMA32 | DMA_MODE_FIFO | DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC;
   
   if(pcm_offset + (88 * 4) > pcm_size){
      //end of pcm buffer
      uint32_t remainder = pcm_size - pcm_offset;
      
      if(loop_audio){
         pcm_offset = 0;
         uint32_t* pcm_32 = (uint32_t*)(pcm_data + pcm_offset);
         for(uint16_t count = 0; count < 88; count++){
            audio_buffer[old_buffer][count] = swap32(pcm_32[count]);
         }
         pcm_offset += 88 * 4;
      }
      else{
         //kill the audio to prevent an overflow
         REG_DMA1CNT = 0;
         
         //disable audio timers
         REG_TM0CNT_H = 0;
         REG_TM1CNT_H = 0;
         irqDisable(IRQ_TIMER1);
      }
   }
   else{
      //copy a full buffer
      uint32_t* pcm_32 = (uint32_t*)(pcm_data + pcm_offset);
      for(uint16_t count = 0; count < 88; count++){
         audio_buffer[old_buffer][count] = swap32(pcm_32[count]);
      }
      pcm_offset += 88 * 4;
   }

}

static void set_play_buffer(int8_t* data, uint32_t freq, uint32_t sample_length){
   pcm_data = data;
   pcm_offset = 0;
   pcm_size = (sample_length / 4) * 4;//cant do non multiple of 4
   
   uint32_t* pcm_32;
   active_buffer = 0;
   
   //fill both buffers
   pcm_32 = (uint32_t*)(pcm_data + pcm_offset);
   for(uint16_t count = 0; count < 88; count++){
      audio_buffer[0][count] = swap32(pcm_32[count]);
   }
   pcm_offset += 88 * 4;
   
   pcm_32 = (uint32_t*)(pcm_data + pcm_offset);
   for(uint16_t count = 0; count < 88; count++){
      audio_buffer[1][count] = swap32(pcm_32[count]);
   }
   pcm_offset += 88 * 4;
   
   REG_SOUNDCNT_L = 0;
   REG_SOUNDCNT_H = SNDA_VOL_100 | SNDA_L_ENABLE | SNDA_R_ENABLE | SNDA_RESET_FIFO;
   //REG_SOUNDCNT_H = 0x0B0F;//unknown, need to decode meaning
   REG_SOUNDCNT_X = SOUND_ENABLE;
   
   //dma_start_addr = (uint32_t) data;//for looping audio
   //REG_DMA1SAD = (uint32_t) data;
   REG_DMA1SAD = (uint32_t) &audio_buffer[0][0];
   REG_DMA1DAD = (uint32_t) &(DSOUND_FIFOA);
   REG_DMA1CNT = DMA_REPEAT | DMA32 | DMA_MODE_FIFO | DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC;
   
   REG_TM0CNT_L = 65536 - (16777216 / freq);
   //REG_TM0CNT_L=0xFBE8; //16khz playback freq
   REG_TM0CNT_H = TIMER_START;
   
   //REG_TM1CNT_L = 65536 - sample_length;
   //REG_TM1CNT_L=0x7098; //0xffff-the number of samples to play
   REG_TM1CNT_L = 0xFFFF - 88 * 4 + 4/*to remove clicks*/; //0xffff-the number of samples to play
   REG_TM1CNT_H = TIMER_CASCADE | TIMER_IRQ | TIMER_START;
   irqEnable(IRQ_TIMER1);
}

void init_audio(){
   loop_audio = false;
   irqSet(IRQ_TIMER1, swap_buffer);
   REG_TM0CNT_H = 0;
}

void play_effect(int8_t* data, uint32_t freq, uint32_t sample_length){
   loop_audio = false;
   set_play_buffer(data, freq, sample_length);
}

void play_music(int8_t* data, uint32_t freq, uint32_t sample_length){
   //loops until told to stop
   loop_audio = true;
   set_play_buffer(data, freq, sample_length);
}

void stop_music(){
   //kill audio
   REG_DMA1CNT = 0;
   
   //disable audio timers
   REG_TM0CNT_H = 0;
   REG_TM1CNT_H = 0;
   irqDisable(IRQ_TIMER1);
}

void play_test(){
   //gba_printf("dma value %08x", DMA_REPEAT | DMA_WORD | DMA_MODE_FIFO | DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC);
   //gba_printf("sound fifo a %08x", (uint32_t) &(DSOUND_FIFOA));
   //gba_printf("sndcnt h %04x", SNDA_VOL_100 | SNDA_L_ENABLE | SNDA_R_ENABLE | SNDA_RESET_FIFO);
   //play_music((int8_t*)smallmiku_raw, 21024, smallmiku_raw_len);
   play_music((int8_t*)smallmiku_clean_raw, 21024, smallmiku_clean_raw_len);
}
