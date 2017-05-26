#include <QCoreApplication>
#include <QAudioDecoder>

#include <vector>
#include <string>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <sstream>

bool fileexists(std::string path){
	struct stat test;
	return (stat(path.c_str(),&test) == 0);
}

std::string uint16_to_hex_string(uint16_t data){
	char temp[50];
	sprintf(temp, "0x%04X", data);
	return temp;
}

std::string format_as_array(std::vector<uint16_t> data){
	std::string output = "uint16_t audio_data[] = {";

	for(size_t index = 0; index < data.size(); index++){
		output += uint16_to_hex_string(data[index]);
		output += " ,";
	}

	output.erase(output.end() - 2, output.end());
	output += "}";
	return output;
}

std::vector<uint16_t> export_16khz_pcm(std::string filename){
	std::vector<uint16_t> pcm_data;

	QAudioFormat desiredFormat;
	desiredFormat.setChannelCount(2);//default 2
	desiredFormat.setCodec("audio/x-raw");
	desiredFormat.setSampleType(QAudioFormat::UnSignedInt);
	desiredFormat.setSampleRate(21024);//default 48000, using obscure gba rate
	desiredFormat.setSampleSize(16);

	QAudioDecoder decoder;
	decoder.setAudioFormat(desiredFormat);
	decoder.setSourceFilename(filename.c_str());//"level1.mp3"

	decoder.start();

	while(!decoder.bufferAvailable());
	QAudioBuffer audio_buffer = decoder.read();
	const quint16 *data = audio_buffer.constData<quint16>();
	int size = audio_buffer.byteCount();

	pcm_data.clear();
	for(int index = 0; index < size; index++){
		pcm_data.push_back(data[index]);
	}

	return pcm_data;
}


int main(int argc, char *argv[])
{
	if(argc > 1){
		if(fileexists(argv[1])){
			std::string clean_data;

			clean_data = format_as_array(export_16khz_pcm(argv[1]));

			//output to file
			std::string output_name = argv[1];
			output_name += ".out";
			std::ofstream ofs(output_name, std::ofstream::out | std::ofstream::trunc);
			ofs << clean_data;
			ofs.close();

			std::cout << clean_data << std::endl;
		}
		else std::cout << "Invalid Params" << std::endl;
	}
	else std::cout << "Invalid Params" << std::endl;
	return 0;
}
