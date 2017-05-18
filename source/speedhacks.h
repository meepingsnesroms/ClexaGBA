#pragma once

#include <stdint.h>

extern "C" void memcpy16(void *dst, const void *src, uint32_t hwcount);
extern "C" void memcpy32(void *dst, const void *src, uint32_t wdcount) IWRAM_CODE;

extern "C" void memset16(void *dst, uint16_t src, uint32_t hwcount);
extern "C" void memset32(void *dst, uint32_t src, uint32_t wdcount) IWRAM_CODE;
