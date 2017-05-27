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

#include "../data/smallmiku.cdata"
//#include "../data/toowak_coats.cdata"

//int8_t* test_data;
//uint32_t test_data_length = 32000;
//uint32_t test_data_length = smallmiku_raw_len;
//uint32_t test_data_freq   = 21024;
//uint32_t test_data_freq   = 16000;
//uint32_t test_data_freq   = 48000;

//this file is based on the example from http://deku.gbadev.org/program/sound2.html

#define SOUND_ENABLE    SNDSTAT_ENABLE
#define TIMER_INTERRUPT TIMER_IRQ
#define TIMER_ENABLE    TIMER_START
#define TIMER_CASCADE   TIMER_COUNT
//#define REG_TM0D        REG_TM0CNT_L
//#define REG_TM1D        REG_TM1CNT_L
#define DMA_WORD        DMA32
#define DMA_MODE_FIFO   DMA_SPECIAL

volatile int8_t*  pcm_data;
volatile uint32_t pcm_offset;
volatile uint32_t pcm_size;
volatile uint8_t  active_buffer;
volatile uint32_t audio_buffer[2][88];//352 bytes, 21024hz

volatile bool     loop_audio;
volatile uint32_t dma_start_addr;

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
   REG_DMA1SAD = (uint32_t)&audio_buffer[active_buffer][0];
   REG_DMA1CNT = DMA_DST_FIXED | DMA_REPEAT | DMA_WORD | DMA_MODE_FIFO | DMA_ENABLE;
   
   if(pcm_offset + 352 > pcm_size){
      //end of pcm buffer
      uint32_t remainder = pcm_size - pcm_offset;
      
      if(loop_audio){
         pcm_offset = 0;
         memcpy32(audio_buffer[old_buffer], pcm_data + pcm_offset, 88);//352 bytes = 88 int32
         for(uint16_t count = 0; count < 88; count++){
            audio_buffer[old_buffer][count] = swap32(audio_buffer[old_buffer][count]);
         }
         pcm_offset += 352;
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
      memcpy32(audio_buffer[old_buffer], pcm_data + pcm_offset, 88);//352 bytes = 88 int32
      for(uint16_t count = 0; count < 88; count++){
         audio_buffer[old_buffer][count] = swap32(audio_buffer[old_buffer][count]);
      }
      pcm_offset += 352;
   }

}

static void block_overrun(){
   if(loop_audio){
      //kill the audio to prevent an overflow
      REG_DMA1CNT = 0;
      
      //restart at the begining
      REG_DMA1SAD = dma_start_addr;
      REG_DMA1CNT = DMA_DST_FIXED | DMA_REPEAT | DMA_WORD | DMA_MODE_FIFO | DMA_ENABLE;
   }
   else{
      //kill the audio to prevent an overflow
      REG_DMA1CNT = 0;
      
      //disable audio timers
      REG_TM0CNT_H = 0;
      REG_TM1CNT_H = 0;
      irqDisable(IRQ_TIMER1);
   }
   
   //REG_IF = IRQ_TIMER1;//clear interrupt
}

static void set_play_buffer(int8_t* data, uint32_t freq, uint32_t sample_length){
   pcm_data = data;
   pcm_offset = 0;
   pcm_size = (sample_length / 4) * 4;//cant do non multiple of 4
   
   active_buffer = 0;
   memcpy32(audio_buffer[0], pcm_data + pcm_offset, 88);//352 bytes = 88 int32
   for(uint16_t count = 0; count < 88; count++){
      audio_buffer[0][count] = swap32(audio_buffer[0][count]);
   }
   pcm_offset += 352;
   
   REG_SOUNDCNT_L = 0;
   REG_SOUNDCNT_H = SNDA_VOL_100 | SNDA_L_ENABLE | SNDA_R_ENABLE | SNDA_RESET_FIFO;
   //REG_SOUNDCNT_H = 0x0B0F;//unknown, need to decode meaning
   REG_SOUNDCNT_X = SOUND_ENABLE;
   
   dma_start_addr = (uint32_t) data;//for looping audio
   REG_DMA1SAD = (uint32_t) data;
   REG_DMA1DAD = (uint32_t) &(DSOUND_FIFOA);
   REG_DMA1CNT = DMA_REPEAT | DMA_WORD | DMA_MODE_FIFO | DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC;
   
   REG_TM0CNT_L = 65536 - (16777216 / freq);
   //REG_TM0CNT_L=0xFBE8; //16khz playback freq
   REG_TM0CNT_H = TIMER_ENABLE;
   
   //REG_TM1CNT_L = 65536 - sample_length;
   //REG_TM1CNT_L=0x7098; //0xffff-the number of samples to play
   REG_TM1CNT_L = 0xFFFF - 88; //0xffff-the number of samples to play
   REG_TM1CNT_H = TIMER_CASCADE | TIMER_INTERRUPT | TIMER_ENABLE;
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
   
}

void play_test(){
   //gba_printf("dma value %08x", DMA_REPEAT | DMA_WORD | DMA_MODE_FIFO | DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC);
   //gba_printf("sound fifo a %08x", (uint32_t) &(DSOUND_FIFOA));
   //gba_printf("sndcnt h %04x", SNDA_VOL_100 | SNDA_L_ENABLE | SNDA_R_ENABLE | SNDA_RESET_FIFO);
   play_music((int8_t*)smallmiku_raw, 21024, smallmiku_raw_len);
   //play_effect
}
