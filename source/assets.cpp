#include <stdint.h>
#include <stdlib.h>//for NULL

#include "gametypes.h"
#include "material.h"

#include "assets.h"

#include "../data/clarke_front.c.out"
#include "../data/clarke_back.c.out"
#include "../data/clarke_left.c.out"
#include "../data/clarke_right.c.out"

#include "../data/smallcrosshair.c.out"//shows where clarkes gun is pointed
#include "../data/crosshair.c.out"

#include "../data/titlescreen.c.out"
#include "../data/flame.c.out"


//item the_flame = {0, 1, {MATTER_SOLID, false, false, true, false, true, 0, (reaction*)NULL}, flame_bitmap, "The Flame"};

/*
item the_flame = {
   .type  = 0; //types are unused currently
   .count = 1;
   .propertys = {
      .matter_state = MATTER_SOLID;
      .flammable    = false;
      .explosive    = false;
      .conductive   = true;
      .edible       = false;
      .radioactive  = true;
      .electric_charge = 0;
   };
   .bitmap = flame_bitmap;
   .name   = "The Flame"
};
*/


const uint16_t rock_tex_bmp[16 * 16] = {
   0xB1AE ,0x9568 ,0xA14A ,0xA54A ,0xA96B ,0xAD8C ,0xA54B ,0xA96B ,0xA96C ,0xA12A ,0xA129 ,0xAD8C ,0x8D47 ,0x88E5 ,0xA129 ,0xA96C ,0xA12A ,0x9D08 ,0xAD8C ,0xB1AE ,0xA54A ,0xBE10 ,0xB9EF ,0x88E5 ,0x8D47 ,0x9D08 ,0xB1AE ,0xB5CE ,0xBE10 ,0xB9EF ,0x8D47 ,0xA54A ,0x9D08 ,0xA54A ,0xB1AE ,0xBE11 ,0xC211 ,0x88E5 ,0x9568 ,0xB1AD ,0x9908 ,0x9568 ,0xB9F0 ,0xC232 ,0xC232 ,0xB5CF ,0x8D47 ,0xA54A ,0xA54A ,0xB1AD ,0xBE11 ,0xC231 ,0xC232 ,0xBE10 ,0xB9EF ,0xB1AE ,0xA96B ,0x9D08 ,0x8D47 ,0xBE10 ,0xBE10 ,0x8D47 ,0xA14A ,0xA529 ,0x9D08 ,0x8D47 ,0xA96C ,0xAD8D ,0xB1CE ,0xAD8D ,0xB5CE ,0x9568 ,0x8D47 ,0x88E5 ,0x8D47 ,0xA96B ,0x88E5 ,0x9D08 ,0xA98C ,0xB1AD ,0xA54B ,0xA129 ,0x9568 ,0x9568 ,0x98E7 ,0x9568 ,0x8D47 ,0xA108 ,0xA54B ,0xB1AD ,0xA54B ,0x8D47 ,0x9D08 ,0xAD8C ,0xB1AE ,0xB1CE ,0xC211 ,0xB9EF ,0xAD8D ,0xA54A ,0x8D47 ,0xA96B ,0x9D08 ,0x8D47 ,0xC232 ,0xC231 ,0xB1CE ,0xB1AD ,0xA52A ,0xB1AE ,0xB5CE ,0xB5CF ,0xB9F0 ,0xB9F0 ,0x8D47 ,0xB1AD ,0xAD8D ,0xA54B ,0xA129 ,0x88E5 ,0xC232 ,0xBE11 ,0xC231 ,0xB9EF ,0x9D08 ,0xB1AE ,0xB9F0 ,0xBE10 ,0xB5CE ,0xB1AE ,0xB1AE ,0x8D47 ,0xA98C ,0x9568 ,0x9568 ,0xA54A ,0xB1AD ,0xB9F0 ,0xB1AE ,0x88E5 ,0x9D08 ,0xA54A ,0xAD8D ,0xBE10 ,0x9568 ,0x9568 ,0xA12A ,0xA12A ,0xA129 ,0xA54A ,0xA96C ,0xA54A ,0x88E5 ,0x8D47 ,0x88E5 ,0x8D47 ,0x8D47 ,0x8D47 ,0xA129 ,0x8D47 ,0xA129 ,0xA54B ,0xA129 ,0x9D08 ,0xA96B ,0x88E5 ,0x8D47 ,0x9568 ,0xB1AE ,0x9568 ,0xA14A ,0xA98C ,0xB1AE ,0xB1AD ,0x88E5 ,0xA54B ,0xB1AE ,0xA98C ,0xB1AE ,0xA54A ,0x9568 ,0xB5CF ,0xBE10 ,0xB9F0 ,0xB1AE ,0xA54B ,0x9568 ,0xBE10 ,0xBE10 ,0xB5EF ,0xB5EF ,0xB5CE ,0xBDF0 ,0xBA10 ,0x9568 ,0x9568 ,0xB9EF ,0xC232 ,0xC231 ,0xBE10 ,0xB1AE ,0xA129 ,0x8D47 ,0xB1AE ,0xC652 ,0xC211 ,0xBE10 ,0xB9F0 ,0xB9F0 ,0x9568 ,0xB1AE ,0xA54A ,0xA96C ,0xBE10 ,0xBE10 ,0xB1CE ,0xA96C ,0x98E7 ,0xA129 ,0x9568 ,0x8D47 ,0xB5CE ,0xBDF0 ,0xB1AE ,0xB1AE ,0x9568 ,0xA96B ,0x9D08 ,0x9568 ,0xB1AE ,0x8D47 ,0xA98C ,0x9D08 ,0x9568 ,0xA54A ,0xA108 ,0x88E5 ,0x8D47 ,0xAD8C ,0xA98C ,0xA96C ,0x9D09 ,0x9568 ,0xA129 ,0xA54A ,0x9D08 ,0xA129 ,0x9D08 ,0x9568 ,0xB18D ,0x9568 ,0x8D47 ,0xA54A ,0xA94B ,0xA129 ,0xA56B
};

texture rock_tex = {16, 16, (uint16_t*)rock_tex_bmp};

uint16_t bullet_color = 0xFEFE;
texture bullet_tex = {1, 1, &bullet_color};
