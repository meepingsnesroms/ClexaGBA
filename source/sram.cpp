#include <gba_base.h>

#define SRAM_END  0xFFFF
#define SRAM_SIZE 0x10000

static uint8_t *const sram = ((uint8_t*)SRAM);

IWRAM_CODE uint8_t read_sram(uint32_t offset){
   return sram[offset];
}

IWRAM_CODE void write_sram(uint32_t offset, uint8_t value){
   sram[offset] = value;
}

void make_valid_checksum(){
   uint32_t checksum = 0;
   uint8_t  bytes[4];
   
   for(uint32_t count = 0; count < SRAM_SIZE - 4; count++){//cant add the checksum to the checksum or it will always be invalid
      checksum += read_sram(count);
   }
   
   bytes[0] = checksum & 0xFF;
   bytes[1] = checksum >> 8 & 0xFF;
   bytes[2] = checksum >> 16 & 0xFF;
   bytes[3] = checksum >> 24 & 0xFF;
   
   write_sram(SRAM_END - 0, bytes[0]);
   write_sram(SRAM_END - 1, bytes[1]);
   write_sram(SRAM_END - 2, bytes[2]);
   write_sram(SRAM_END - 3, bytes[3]);
}

bool is_data_corrupt(){
   uint32_t checksum = 0;
   uint32_t compare = read_sram(SRAM_END - 3) << 24 | read_sram(SRAM_END - 2) << 16 | read_sram(SRAM_END - 1) << 8 | read_sram(SRAM_END - 0);
   
   if(compare == 'PASS'){
      //the data is user edited, disable checksum
      return false;//data is from user, assume its valid
   }
   
   for(uint32_t count = 0; count < SRAM_SIZE - 4; count++){//cant add the checksum to the checksum or it will always be invalid
      checksum += read_sram(count);
   }
   
   if(checksum == compare){
      return false;//data intact
   }
   return true;//invalid data, checksum not equal
}

void format_save(){
   for(uint32_t count = 0; count < SRAM_SIZE; count++){
      write_sram(count, 0xFF);
   }
   make_valid_checksum();
}
