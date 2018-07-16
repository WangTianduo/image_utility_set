#include "imgStruct.h"
#include <stdint.h>
#include <fstream>

imgUtil::ManagedBlob imgUtil::read_content_from_file(const char* path) {
	std::ifstream ifile;
	ifile.open(path, std::ios::binary);

	if (!ifile) {
		return {};
	}

	ManagedBlob ret;
	ifile.seekg(0, ifile.end); //put the ptr in the end
	size_t length = ifile.tellg(); //get the pos of ptr, same as get the length
	ret.size = length;
	ret.buffer.reset(new uint8_t[length]);
	ifile.seekg(0, ifile.beg);
	ifile.read(reinterpret_cast<char*>(ret.buffer.get()), length);
	return ret;
}
