#include <iostream>
#include <cstdint>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

using namespace std;

#define RGB8(r,g,b)	( (((b)>>3)<<10) | (((g)>>3)<<5) | ((r)>>3) ) //taken from libgba
#define TOUPPERCASE(x) ((x) - 0x20)

bool fileexists(string path){
	struct stat test;
	return (stat(path.c_str(),&test) == 0);
}

string color_string(uint16_t color){
   char temp[50];
   sprintf(temp, "0x%04X", color);
   return temp;
}

uint16_t fix_32bit_color(uint32_t old_color){
   if(old_color == 0x00000000)return 0x0000;
   uint8_t r = (old_color >> 16) & 0xFF;
   uint8_t g = (old_color >> 8) & 0xFF;
   uint8_t b = old_color & 0xFF;
   uint16_t finished = RGB8(b,g,r) | 0x8000/*visible bit*/;
   return finished;
}

string parse(string data){
   string output;
   uint64_t offset = 0;
   
   while(1){
	  offset = data.find("0x",offset + 10);
      if(offset == string::npos)break;//ran out of pixels
	  //cout << data.substr(offset + 2,8) << endl;
	  uint32_t color_data = std::stoul(data.substr(offset + 2,8),nullptr,16);
      uint16_t fixed_color = fix_32bit_color(color_data);
      output += color_string(fixed_color);
      output += " ,";
   }
   
   return output;
}

int main(int argc, char *argv[])
{
	if(argc > 1){
		if(fileexists(argv[1])){
			std::ifstream infile(argv[1]);
			std::stringstream buffer;
			buffer << infile.rdbuf();
			string cleandata = parse(buffer.str());
			cout << cleandata << endl;
		}
	}else cout << "Invalid Prams" << endl;
	return 0;
}
