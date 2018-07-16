#include <iostream>
#include <memory>
#include "jpeg_core.h"
#include "format_change.h"
#include "jpeglib.h"
#include <fstream>

int main() {
	imgUtil::ManagedImage imgInput;
	imgUtil::jpeg2rgb(imgInput, "qq.jpg");

	imgUtil::OutputImage imgOutput;

	unsigned char* src = imgInput.data.buffer.get();
	unsigned char *dst = (unsigned char*)malloc(imgInput.data.size);

	int outH = 0;
	int outW = 0;

	imgUtilTool::BaseFormatTransform::rotate_rgb(src, imgInput.width, imgInput.height, 90, dst, outW, outH);
	imgOutput.height = outH;
	imgOutput.width = outW;
	imgOutput.data.buffer = dst;

	imgUtil::rgb2jpeg(imgOutput, "qq2.jpg");
	return 0;
}

