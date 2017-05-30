#pragma once

#include <stdint.h>

#include "gametypes.h"
#include "material.h"

#define ROM_DATA __attribute__((section(".text")))

extern uint16_t clarke_front_data[];
extern uint16_t clarke_back_data[];
extern uint16_t clarke_left_data[];
extern uint16_t clarke_right_data[];

extern uint16_t flame_data[];
extern uint16_t title_screen_data[];

extern texture  rock_tex;
