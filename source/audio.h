#pragma once

#include <stdint.h>

void init_audio();
void play_effect(int8_t* data, uint32_t freq, uint32_t sample_length);
void play_music(int8_t* data, uint32_t freq, uint32_t sample_length);//loops until told to stop
void stop_music();

void play_test();//test function, plays a test noise
