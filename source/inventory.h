#pragma once

#include "itemmenu.h"

void init_inventory();
void default_inventory();
item* open_inventory();//returns item if one is selected NULL if menu was closed

void add_item(item& new_item);
