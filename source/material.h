#pragma once

#include <stdint.h>

#include "gametypes.h"

//all item bitmaps should be 50x50

enum{
   MATTER_SOLID,
   MATTER_LIQUID,
   MATTER_GAS,
   MATTER_PLASMA
};

typedef struct{
   uint16_t reagent;
   uint16_t result;
}reaction;

typedef struct{
   uint8_t matter_state;
   bool flammable;
   bool explosive;
   bool conductive;
   bool edible;
   bool radioactive;
   int8_t electric_charge;
   reaction* reactions;
}material;

typedef struct{
   uint16_t type;
   uint16_t count;
   material propertys;
   texture item_image;
   char name[20];
}item;
