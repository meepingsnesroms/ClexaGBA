#pragma once

#include <stdint.h>

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
