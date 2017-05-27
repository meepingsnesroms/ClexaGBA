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

//#include "../data/smallmiku.cdata"
#include "../data/toowak_coats.cdata"

int8_t* test_data;
uint32_t test_data_length = 32000;
//uint32_t test_data_length = smallmiku_raw_len;
uint32_t test_data_freq   = 21024;
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

volatile bool     loop_audio;
volatile uint32_t dma_start_addr;

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

void retard_audio(int8_t* swap, uint32_t size){
   //gba has uint32_t[4] to store 16 audio samples, it is also little endian so uint8_t[16] is not compatible
   //order samples are stored in bytes uint8_t[16]{3,2,1,0,7,6,5,4,...}
   
   size /= 4;
   int8_t storage[4];
   for(uint32_t count = 0; count < size; count++){
      uint32_t offset = count * 4;
      storage[0] = swap[offset];
      storage[1] = swap[offset + 1];
      storage[2] = swap[offset + 2];
      storage[3] = swap[offset + 3];
      
      swap[offset + 3] = storage[0];
      swap[offset + 2] = storage[1];
      swap[offset + 1] = storage[2];
      swap[offset] = storage[3];
   }
}

static void set_play_buffer(int8_t* data, uint32_t freq, uint32_t sample_length){
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
   REG_TM1CNT_L=0x7098; //0xffff-the number of samples to play
   REG_TM1CNT_H = TIMER_CASCADE | TIMER_INTERRUPT | TIMER_ENABLE;
   irqEnable(IRQ_TIMER1);
}

void init_audio(){
   loop_audio = false;
   irqSet(IRQ_TIMER1, block_overrun);
   REG_TM0CNT_H = 0;
   
   //temp
   test_data = (int8_t*)malloc(40000);
   if(test_data == NULL){
      bsod("Cant get audio buffer.");
   }
   
   memcpy(test_data, smallmiku_raw, 40000);
   //retard_audio(test_data, test_data_length);
   retard_audio(test_data, 40000);
   test_data_length = 40000;
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
   play_music(test_data, test_data_freq, test_data_length);
}
