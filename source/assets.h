#pragma once

#include <stdint.h>

#include "gametypes.h"
#include "material.h"

//forces data to be stored in rom instead of unpacking it to ram on launch
#define ROM_DATA __attribute__((section(".text")))

//images
extern uint16_t clarke_front_data[];
extern uint16_t clarke_back_data[];
extern uint16_t clarke_left_data[];
extern uint16_t clarke_right_data[];

extern uint16_t smallcrosshair_data[];
extern uint16_t crosshair_data[];

extern uint16_t flame_data[];
extern uint16_t title_screen_data[];

extern texture  rock_tex;

//levels
