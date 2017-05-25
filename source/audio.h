#pragma once

#include <stdint.h>

void init_audio();
void play_effect(int16_t* data, uint32_t freq, uint32_t sample_length);
void play_music(int16_t* data, uint32_t freq, uint32_t sample_length);//loops until told to stop
void stop_music();
