#include <gba_sound.h>
#include <gba_timers.h>
#include <gba_interrupt.h>
#include <gba_dma.h>

#include <stdint.h>

int16_t test_data[32] = {-0x7FFF, 0x0000, -0x7FFF, 0x0000, -0x7FFF, 0x0000, -0x7FFF, 0x0000,
   -0x7FFF, 0x0000, -0x7FFF, 0x0000, -0x7FFF, 0x0000, -0x7FFF, 0x0000,
   -0x7FFF, 0x0000, -0x7FFF, 0x0000, -0x7FFF, 0x0000, -0x7FFF, 0x0000,
   -0x7FFF, 0x0000, -0x7FFF, 0x0000, -0x7FFF, 0x0000, -0x7FFF, 0x0000};

uint32_t test_data_length = 32;
uint32_t test_data_freq   = 16000;

//this file is based on the example from http://deku.gbadev.org/program/sound2.html

#define SOUND_ENABLE    SNDSTAT_ENABLE
#define TIMER_INTERRUPT TIMER_IRQ
#define TIMER_ENABLE    TIMER_START
#define TIMER_CASCADE   TIMER_COUNT
//#define REG_TM0D        REG_TM0CNT_L
//#define REG_TM1D        REG_TM1CNT_L
#define DMA_WORD        DMA32
#define DMA_MODE_FIFO   DMA_SPECIAL

volatile bool loop_audio;

static void block_overrun(){
   if(loop_audio){
      //kill the audio to prevent an overflow
      REG_DMA1CNT = 0;
      //restart at the begining
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

static void set_play_buffer(int16_t* data, uint32_t freq, uint32_t sample_length){
   REG_SOUNDCNT_H = SNDA_VOL_100 | SNDA_L_ENABLE | SNDA_R_ENABLE | SNDA_RESET_FIFO;
   REG_SOUNDCNT_X = SOUND_ENABLE;
   
   REG_TM0CNT_L = 65536 - (16777216 / freq);
   REG_TM0CNT_H = TIMER_ENABLE;
   
   REG_DMA1SAD = (uint32_t) data;
   REG_DMA1DAD = (uint32_t) &(DSOUND_FIFOA);
   REG_DMA1CNT = DMA_DST_FIXED | DMA_REPEAT | DMA_WORD | DMA_MODE_FIFO | DMA_ENABLE;
   
   REG_TM1CNT_L = 65536 - sample_length;
   REG_TM1CNT_H = TIMER_CASCADE | TIMER_INTERRUPT | TIMER_ENABLE;
   irqEnable(IRQ_TIMER1);
}

void init_audio(){
   loop_audio = false;
   irqSet(IRQ_TIMER1, block_overrun);
}

void play_effect(int16_t* data, uint32_t freq, uint32_t sample_length){
   loop_audio = false;
   set_play_buffer(data, freq, sample_length);
}

void play_music(int16_t* data, uint32_t freq, uint32_t sample_length){
   //loops until told to stop
   loop_audio = true;
   set_play_buffer(data, freq, sample_length);
}

void stop_music(){
   
}

void play_test(){
   play_music(test_data, test_data_freq, test_data_length);
}
