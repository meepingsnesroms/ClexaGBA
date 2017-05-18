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

void better_replace(string& str, uint64_t start, string to_find, string replacement){
	str.replace(str.find(to_find,start), to_find.length(), replacement);
}

string make_header(string data){
	string working_data;
	uint64_t first_bracket = 0;

	//extract old header
	first_bracket = data.find("{",first_bracket);
	if(first_bracket == string::npos)abort();
	working_data = data.substr(0,first_bracket);

	//patch header
	better_replace(working_data, 0, "uint32_t", "uint16_t");
	better_replace(working_data, 0, "static", "/*static*/");
	better_replace(working_data, 0, "_data[1]", "_data");

	return working_data;
}

string extract_data(string data){
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
   
   //remove the " ," on the last entry
   output.erase(output.end() - 2, output.end());

   return output;
}

string make_16bit_array(string data){
	string output;

	output += make_header(data);
	output += "{\n";//start array
	output += extract_data(data);
	output += "\n};";//close array

	return output;
}

int main(int argc, char *argv[])
{
	if(argc > 1){
		if(fileexists(argv[1])){
			std::ifstream infile(argv[1]);
			std::stringstream buffer;
			buffer << infile.rdbuf();
			string cleandata = make_16bit_array(buffer.str());

			//output to file
			string output_name = argv[1];
			output_name += ".out";
			std::ofstream ofs(output_name, std::ofstream::out | std::ofstream::trunc);
			ofs << cleandata;
			ofs.close();

			cout << cleandata << endl;
		}
	}else cout << "Invalid Prams" << endl;
	return 0;
}
