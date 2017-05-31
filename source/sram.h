#pragma once

#include <gba_base.h>

#include <stdint.h>

IWRAM_CODE uint8_t read_sram(uint32_t offset);
IWRAM_CODE void write_sram(uint32_t offset, uint8_t value);

void make_valid_checksum();//sets sram checksum to checksum of existing data
bool is_data_corrupt();//verifys the checksum

void format_save();
