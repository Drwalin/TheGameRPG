#pragma once

#include <string>

#include "../../ICon7/include/icon7/Forward.hpp"

class FileOperations
{
public:
	static bool ReadFile(std::string filePath,
						 icon7::ByteBufferWritable *buffer);
	static bool WriteFile(std::string filePath,
						  const icon7::ByteBufferWritable &buffer);
	static bool FileExists(std::string filePath);
};
