#pragma once
#include "imgStruct.h"

namespace imgUtil {
	//output img as jpeg
	int rgb2jpeg(OutputImage &out_image, const char *jpeg_file);
	int yuvnv122jpeg(OutputImage &out_image, const char *jpeg_file);
	int yuvnv212jpeg(OutputImage &out_image, const char *jpeg_file);
	//input img from jpeg
	int jpeg2rgb(ManagedImage &image, const char *jpeg_file);
	int jpeg2yuvnv21(ManagedImage &image, const char *jpeg_file);
	int jpeg2yuvnv12(ManagedImage &image, const char *jpeg_file);
	//form conversion
	//bool yuvnv122RGB(unsigned char* yuvnv21_buffer, unsigned char* rgb_buffer, int width, int height);
	//bool yuvnv212RGB(unsigned char* yuvnv21_buffer, unsigned char* rgb_buffer, int width, int height);
}
