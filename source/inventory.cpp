#include "itemmenu.h"
#include "assets.h"

#include <string.h>//strcat, strcpy
#include <stdlib.h>//itoa

#define MAX_HELD_ITEMS 20

item     currently_held_items[MAX_HELD_ITEMS];
item     formatted_item_list[MAX_HELD_ITEMS];
uint32_t total_items;

static char make_lowercase(char chr){
   //leave lowercase alone
   //leave numbers alone
   if(chr >= 'A' && chr <= 'Z'){
      chr += 0x20;
   }
   return chr;
}

static void sort_by_letter(){
   
}

static void make_menu_readable_item_list(){
   
}

uint32_t get_last_item(){
   for(int32_t cnt = MAX_HELD_ITEMS - 1; cnt > -1; cnt++){
      if(currently_held_items[cnt].active){
         return cnt;
      }
   }
}

void fill_hole(uint32_t hole){
   //replaces index at hole with last item
   uint32_t last_item = get_last_item();
   currently_held_items[hole] = currently_held_items[last_item];
   currently_held_items[last_item].active = false;
}

void add_item(item& new_item){
   currently_held_items[total_items] = new_item;
   total_items++;
}


void remove_item(uint32_t index){
   currently_held_items[index].active = false;
   if(index != total_items - 1){
      fill_hole(index);
      total_items--;
   }
}

void default_inventory(){
   item the_flame;
   
   the_flame.type  = 0; //types are unused currently
   the_flame.count = 1;
   the_flame.active = true;
   
   the_flame.propertys.matter_state = MATTER_SOLID;
   the_flame.propertys.flammable    = false;
   the_flame.propertys.explosive    = false;
   the_flame.propertys.conductive   = true;
   the_flame.propertys.edible       = false;
   the_flame.propertys.radioactive  = true;
   the_flame.propertys.electric_charge = 0;
   
   the_flame.item_image.w = 43;
   the_flame.item_image.h = 60;
   the_flame.item_image.bitmap = flame_data;
   strcpy(the_flame.name, "The Flame");
   
   add_item(the_flame);
}


void init_inventory(){
   total_items = 0;
   default_inventory();
   
#if 1//test items
   item* test_items = currently_held_items;
   total_items = MAX_HELD_ITEMS;
   char data_str[20];
   for(uint32_t cnt = 1; cnt < MAX_HELD_ITEMS; cnt++){
      strcpy(test_items[cnt].name, "FkItem");
      strcat(test_items[cnt].name, itoa(cnt, data_str, 10));
      test_items[cnt].item_image.bitmap = NULL;
   }
#endif
}

item* open_inventory(){
   return list_items(currently_held_items, total_items, true /*exit_allowed*/);
}
