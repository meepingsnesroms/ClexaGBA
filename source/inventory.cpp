#include "itemmenu.h"

#include <string.h>//strcat, strcpy
#include <stdlib.h>//itoa

item currently_held_items[20];

void open_inventory(){
#if 1//test items
   item test_items[20];
   char data_str[20];
   for(uint8_t cnt = 0; cnt < 20; cnt++){
      strcpy(test_items[cnt].name, "FkItem");
      strcat(test_items[cnt].name, itoa(cnt, data_str, 10));
   }
   list_items(test_items);
#endif
   //list_items(currently_held_items);
}
