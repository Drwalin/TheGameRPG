#include <cstdio>

#include <icon7/Debug.hpp>

#include "../include/FileOperations.hpp"

bool FileOperations::ReadFile(std::string filePath, icon7::ByteBuffer *buffer)
{
	FILE *file = fopen(filePath.c_str(), "rb");
	if (buffer->valid() == false) {
		buffer->Init(4096);
	}
	if (file) {
		const uint32_t COUNT = 4096;
		while (!feof(file)) {
			uint32_t oldSize = buffer->size();
			buffer->resize(oldSize + COUNT);
			int r = fread(buffer->data() + oldSize, 1, COUNT, file);
			buffer->resize(oldSize + r);
			if (r == 0) {
				LOG_ERROR("ERR");
				break;
			}
		}
	}
	if (file) {
		fclose(file);
		return true;
	}
	return false;
}

bool FileOperations::WriteFile(std::string filePath,
							   const icon7::ByteBuffer &buffer)
{
	FILE *file = fopen(filePath.c_str(), "wb");
	if (file) {
		bool result = true;
		uint32_t written = 0;
		while (written < buffer.size()) {
			int b = fwrite(buffer.data() + written, 1, buffer.size() - written,
						   file);
			if (b <= 0) {
				LOG_ERROR(
					"Error saving player data to file, fwrite returned: %i", b);
				result = false;
				break;
			}
			written += b;
		}
		fclose(file);
		return result;
	}
				LOG_ERROR("ERR");
	return false;
}
