#pragma once

#include "material.h"

typedef struct{
   uint16_t type;
   uint16_t count;
   material propertys;
   uint16_t bitmap;
   char name[20];
}item;


item* list_items(item* items);
