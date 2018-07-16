#pragma once
#include <memory>
#include <stdlib.h>
#include <iostream>

namespace imgUtil {

	struct ManagedBlob {
        std::unique_ptr<uint8_t[]> buffer;
		size_t size = 0;

		void resize(size_t sz) {
			if (sz > size) {
				size = sz;
				buffer.reset(new uint8_t[size]);
			}
			else if (sz < size) {
				size = sz;
			}
		}
	};

	struct OutputBuf {
		unsigned char* buffer;
		size_t size = 0;
	};

	typedef struct ManagedImage {
		ManagedBlob data; // data contains both buffer and size
		int width = -1, height = -1;
	};// both i/o img will use this struct, wait to see if there is any error without two

	typedef struct OutputImage {
		OutputBuf data;
		int width = -1, height = -1;
	};

	ManagedBlob read_content_from_file(const char* path);

}
