#include <iostream>
#include <memory>
#include "jpeg_core.h"
#include "jpeglib.h"
#include <fstream>
#include <cstring>

//using namespace imgUtil;

int imgUtil::rgb2jpeg(OutputImage &img, const char *jpeg_file) {
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	FILE *outfile;
	JSAMPROW row_pointer[1];
	int row_stride;

	int image_width = img.width;
	int image_height = img.height;
	unsigned char* bgr_buffer = img.data.buffer;//(unsigned char *)out_image.data.buffer.get();

	JSAMPLE *image_buffer = (JSAMPLE *)bgr_buffer;
	int image_buffer_len = image_width * image_height * 3;

	if (!(outfile = fopen(jpeg_file, "wb"))) {
		fprintf(stderr, "can't open %s\n", jpeg_file);
		return -1;
	}
	else {
		std::unique_ptr<FILE, int(*)(FILE *)> fp{ outfile, fclose };

		cinfo.err = jpeg_std_error(&jerr);

		jpeg_create_compress(&cinfo);

		jpeg_stdio_dest(&cinfo, fp.get());


		cinfo.image_width = image_width;
		cinfo.image_height = image_height;
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;

		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, 100, TRUE);
		jpeg_start_compress(&cinfo, TRUE);


		row_stride = cinfo.image_width * 3;
		while (cinfo.next_scanline < cinfo.image_height) {
			row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}

		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);
		return 1;
	}
}

int imgUtil::jpeg2rgb(ManagedImage &image, const char *jpeg_file) {
	ManagedBlob blob = read_content_from_file(jpeg_file);
	if (blob.size <= 0) return 0;

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, blob.buffer.get(), blob.size);

	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	unsigned long width = cinfo.output_width;
	unsigned long height = cinfo.output_height;
	unsigned short depth = cinfo.output_components;

	//image.type = MG_HUM_IMG_BGR;
	image.width = width;
	image.height = height;
	image.data.resize(width*height*depth);
	image.data.size = width*height*depth;

	uint8_t *bgr = image.data.buffer.get();
	memset(bgr, 0, width*height*depth);

	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, width*depth, 1);

	uint8_t *point = bgr;
	while (cinfo.output_scanline<height)
	{
		jpeg_read_scanlines(&cinfo, buffer, 1);
		memcpy(point, *buffer, width*depth);
		point += width*depth;
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return 1;
}

static long int crv_tab[256];
static long int cbu_tab[256];
static long int cgu_tab[256];
static long int cgv_tab[256];
static long int tab_76309[256];
static unsigned char clp[1024];

void init_yuv420p_table()
{
	long int crv, cbu, cgu, cgv;
	int i, ind;
	static int init = 0;

	if (init == 1) return;

	crv = 104597; cbu = 132201;  /* fra matrise i global.h */
	cgu = 25675;  cgv = 53279;

	for (i = 0; i < 256; i++)
	{
		crv_tab[i] = (i - 128) * crv;
		cbu_tab[i] = (i - 128) * cbu;
		cgu_tab[i] = (i - 128) * cgu;
		cgv_tab[i] = (i - 128) * cgv;
		tab_76309[i] = 76309 * (i - 16);
	}

	for (i = 0; i < 384; i++)
		clp[i] = 0;
	ind = 384;
	for (i = 0; i < 256; i++)
		clp[ind++] = i;
	ind = 640;
	for (i = 0; i < 384; i++)
		clp[ind++] = 255;

	init = 1;
}


bool yuvnv122RGB(unsigned char* yuvnv21_buffer, unsigned char* rgb_buffer, int width, int height) {
	if (width < 1 || height < 1 || yuvnv21_buffer == NULL || rgb_buffer == NULL)
		return false;

	int y1, y2, u, v;
	unsigned char *py1, *py2;
	int i, j, c1, c2, c3, c4;
	unsigned char *d1, *d2;
	unsigned char *src_u;
	static int init_yuv420p = 0;

	src_u = yuvnv21_buffer + width * height;   // u

	py1 = yuvnv21_buffer;   // y
	py2 = py1 + width;
	d1 = rgb_buffer;
	d2 = d1 + 3 * width;

	init_yuv420p_table();

	for (j = 0; j < height; j += 2) {
		for (i = 0; i < width; i += 2) {
			u = *src_u++;      // u紧跟v，在v的下一个位置
			v = *src_u++;

			c1 = crv_tab[v];
			c2 = cgu_tab[u];
			c3 = cgv_tab[v];
			c4 = cbu_tab[u];

			//up-left
			y1 = tab_76309[*py1++];

			*d1++ = clp[384 + ((y1 + c1) >> 16)];
			*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
			*d1++ = clp[384 + ((y1 + c4) >> 16)];

			//down-left
			y2 = tab_76309[*py2++];
			*d2++ = clp[384 + ((y2 + c1) >> 16)];
			*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];

			*d2++ = clp[384 + ((y2 + c4) >> 16)];
			//up-right
			y1 = tab_76309[*py1++];
			*d1++ = clp[384 + ((y1 + c1) >> 16)];
			*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];

			*d1++ = clp[384 + ((y1 + c4) >> 16)];
			//down-right
			y2 = tab_76309[*py2++];
			*d2++ = clp[384 + ((y2 + c1) >> 16)];
			*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
			*d2++ = clp[384 + ((y2 + c4) >> 16)];
		}
		d1 += 3 * width;
		d2 += 3 * width;
		py1 += width;
		py2 += width;
	}

	return true;
}

int imgUtil::yuvnv122jpeg(OutputImage &out_image, const char *jpeg_file) {
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	FILE *outfile;
	JSAMPROW row_pointer[1];
	int row_stride;

	int image_width = out_image.width;
	int image_height = out_image.height;
	std::fstream file;
	unsigned char* rgb_buffer = (unsigned char *)malloc(image_width * image_height * 3);
	bool isToRGB = yuvnv122RGB((unsigned char *)out_image.data.buffer, rgb_buffer, image_width, image_height);
	file.open("./test_photo/tmp.bgr", std::ios::binary | std::ios::out);
	file.write((const char *)rgb_buffer, image_width * image_height * 3);
	file.close();
	if (isToRGB == false) {
		return 0;
	}

	JSAMPLE *image_buffer = (JSAMPLE *)rgb_buffer;
	int image_buffer_len = image_width * image_height * 3;

	if (!(outfile = fopen(jpeg_file, "wb"))) {
		free(rgb_buffer);
		fprintf(stderr, "can't open %s\n", jpeg_file);
		return -1;
	}
	else {
		std::unique_ptr<FILE, int(*)(FILE *)> fp{ outfile, fclose };

		cinfo.err = jpeg_std_error(&jerr);

		jpeg_create_compress(&cinfo);

		jpeg_stdio_dest(&cinfo, fp.get());


		cinfo.image_width = image_width;
		cinfo.image_height = image_height;
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;

		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, 100, TRUE);
		jpeg_start_compress(&cinfo, TRUE);


		row_stride = cinfo.image_width * 3;
		while (cinfo.next_scanline < cinfo.image_height) {
			row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}

		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);
		free(rgb_buffer);
		return 1;
	}
}

bool yuvnv212RGB(unsigned char* yuvnv21_buffer, unsigned char* rgb_buffer, int width, int height) {
	if (width < 1 || height < 1 || yuvnv21_buffer == NULL || rgb_buffer == NULL)
		return false;

	int y1, y2, u, v;
	unsigned char *py1, *py2;
	int i, j, c1, c2, c3, c4;
	unsigned char *d1, *d2;
	unsigned char *src_u;
	static int init_yuv420p = 0;

	src_u = yuvnv21_buffer + width * height;   // u

	py1 = yuvnv21_buffer;   // y
	py2 = py1 + width;
	d1 = rgb_buffer;
	d2 = d1 + 3 * width;

	init_yuv420p_table();

	for (j = 0; j < height; j += 2) {
		for (i = 0; i < width; i += 2) {
			v = *src_u++;
			u = *src_u++;      // u紧跟v，在v的下一个位置

			c1 = crv_tab[v];
			c2 = cgu_tab[u];
			c3 = cgv_tab[v];
			c4 = cbu_tab[u];

			//up-left
			y1 = tab_76309[*py1++];
			*d1++ = clp[384 + ((y1 + c1) >> 16)];
			*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
			*d1++ = clp[384 + ((y1 + c4) >> 16)];

			//down-left
			y2 = tab_76309[*py2++];
			*d2++ = clp[384 + ((y2 + c1) >> 16)];
			*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
			*d2++ = clp[384 + ((y2 + c4) >> 16)];

			//up-right
			y1 = tab_76309[*py1++];
			*d1++ = clp[384 + ((y1 + c1) >> 16)];
			*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
			*d1++ = clp[384 + ((y1 + c4) >> 16)];

			//down-right
			y2 = tab_76309[*py2++];
			*d2++ = clp[384 + ((y2 + c1) >> 16)];
			*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
			*d2++ = clp[384 + ((y2 + c4) >> 16)];
		}
		d1 += 3 * width;
		d2 += 3 * width;
		py1 += width;
		py2 += width;
	}

	return true;
}


int imgUtil::yuvnv212jpeg(OutputImage &out_image, const char *jpeg_file) {
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	FILE *outfile;
	JSAMPROW row_pointer[1];
	int row_stride;

	int image_width = out_image.width;
	int image_height = out_image.height;
	std::fstream file;
	unsigned char* rgb_buffer = (unsigned char *)malloc(image_width * image_height * 3);
	bool isToRGB = yuvnv212RGB((unsigned char *)out_image.data.buffer, rgb_buffer, image_width, image_height);
	file.open("./test_photo/tmp.bgr", std::ios::binary | std::ios::out);
	file.write((const char *)rgb_buffer, image_width * image_height * 3);
	file.close();
	if (isToRGB == false) {
		return 0;
	}

	JSAMPLE *image_buffer = (JSAMPLE *)rgb_buffer;
	int image_buffer_len = image_width * image_height * 3;

	if (!(outfile = fopen(jpeg_file, "wb"))) {
		free(rgb_buffer);
		fprintf(stderr, "can't open %s\n", jpeg_file);
		return -1;
	}
	else {
		std::unique_ptr<FILE, int(*)(FILE *)> fp{ outfile, fclose };

		cinfo.err = jpeg_std_error(&jerr);

		jpeg_create_compress(&cinfo);

		jpeg_stdio_dest(&cinfo, fp.get());


		cinfo.image_width = image_width;
		cinfo.image_height = image_height;
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;

		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, 100, TRUE);
		jpeg_start_compress(&cinfo, TRUE);


		row_stride = cinfo.image_width * 3;
		while (cinfo.next_scanline < cinfo.image_height) {
			row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}

		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);
		free(rgb_buffer);
		return 1;
	}
};

int imgUtil::jpeg2yuvnv21(ManagedImage &image, const char *jpeg_file) {
	ManagedImage rgb_image;
	jpeg2rgb(rgb_image, jpeg_file);

	//image.type = MG_HUM_IMG_NV21;
	image.width = rgb_image.width;
	image.height = rgb_image.height;
	int image_size = image.width * image.height * 3 / 2;
	image.data.resize(image_size);
	image.data.size = image_size;

	int y, u, v;
	int nv21_len = image.width * image.height;
	unsigned char* nv21_buffer = image.data.buffer.get();
	memset(nv21_buffer, 0, image_size);
	unsigned char* rgb_buffer = (unsigned char *)rgb_image.data.buffer.get();
	for (int i = 0; i < image.height; i++) {
		for (int j = 0; j < image.width; j++) {
			int currentR = rgb_buffer[(i * image.width + j) * 3 + 0];
			int currentG = rgb_buffer[(i * image.width + j) * 3 + 1];
			int currentB = rgb_buffer[(i * image.width + j) * 3 + 2];

			y = ((66 * currentR + 129 * currentG + 25 * currentB + 128) >> 8) + 16;
			u = ((-38 * currentR - 74 * currentG + 112 * currentB + 128) >> 8) + 128;
			v = ((112 * currentR - 94 * currentG - 18 * currentB + 128) >> 8) + 128;
			y = y < 16 ? 16 : (y > 255 ? 255 : y);
			u = u < 0 ? 0 : (u > 255 ? 255 : u);
			v = v < 0 ? 0 : (v > 255 ? 255 : v);

			nv21_buffer[i * image.width + j] = y;
			nv21_buffer[nv21_len + (i >> 1) * image.width + (j & ~1) + 0] = v;
			nv21_buffer[nv21_len + +(i >> 1) * image.width + (j & ~1) + 1] = u;
		}
	}
	return 1;
}

int imgUtil::jpeg2yuvnv12(ManagedImage &image, const char *jpeg_file) {
	ManagedImage rgb_image;
	jpeg2rgb(rgb_image, jpeg_file);

	//image.type = MG_HUM_IMG_NV21;
	image.width = rgb_image.width;
	image.height = rgb_image.height;
	int image_size = image.width * image.height * 3 / 2;
	image.data.resize(image_size);
	image.data.size = image_size;

	int y, u, v;
	int nv12_len = image.width * image.height;
	unsigned char* nv12_buffer = image.data.buffer.get();
	memset(nv12_buffer, 0, image_size);
	unsigned char* rgb_buffer = (unsigned char *)rgb_image.data.buffer.get();
	for (int i = 0; i < image.height; i++) {
		for (int j = 0; j < image.width; j++) {
			int currentR = rgb_buffer[(i * image.width + j) * 3 + 0];
			int currentG = rgb_buffer[(i * image.width + j) * 3 + 1];
			int currentB = rgb_buffer[(i * image.width + j) * 3 + 2];

			y = ((66 * currentR + 129 * currentG + 25 * currentB + 128) >> 8) + 16;
			u = ((-38 * currentR - 74 * currentG + 112 * currentB + 128) >> 8) + 128;
			v = ((112 * currentR - 94 * currentG - 18 * currentB + 128) >> 8) + 128;
			y = y < 16 ? 16 : (y > 255 ? 255 : y);
			u = u < 0 ? 0 : (u > 255 ? 255 : u);
			v = v < 0 ? 0 : (v > 255 ? 255 : v);

			nv12_buffer[i * image.width + j] = y;
			nv12_buffer[nv12_len + (i >> 1) * image.width + (j & ~1) + 0] = u;
			nv12_buffer[nv12_len + +(i >> 1) * image.width + (j & ~1) + 1] = v;
		}
	}
	return 1;
}


/*int main() {
	imgUtil::ManagedImage imgInput;
	imgUtil::jpeg2rgb(imgInput, "qq.jpg");

	imgUtil::ManagedImage imgOutput;

	imgOutput.width = imgInput.width;
	imgOutput.height = imgInput.height;
	imgOutput.data.size = imgInput.data.size;
	imgOutput.data.buffer = std::move(imgInput.data.buffer);
	/*imgUtil::rgb2jpeg(imgInput.width, imgInput.height, imgInput.data.size,
		(unsigned char*)imgInput.data.buffer.get(), "C:\\Users\\User\\Desktop\\ImageUtil\\imgUtil0622\\imgUtil0622\\qq2.jpg");*/
	/*imgUtil::rgb2jpeg(imgOutput, "C:\\Users\\User\\Desktop\\ImageUtil\\imgUtil0622\\imgUtil0622\\qq2.jpg");
	return 0;
}*/
