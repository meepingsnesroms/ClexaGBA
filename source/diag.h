#pragma once

#include <stdarg.h>
#include <stdint.h>

typedef struct{
   bool (*test_func)(void);
   char name[50];
}diag_test;
extern char test_result[200];

void wait_for_key_press(uint16_t key);

void gba_printf(char* str, ...);
void run_tests();
