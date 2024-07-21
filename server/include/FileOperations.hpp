#pragma once

#include <string>

#include <icon7/Forward.hpp>

class FileOperations
{
public:
	static bool ReadFile(std::string filePath, icon7::ByteBuffer *buffer);
	static bool WriteFile(std::string filePath,
						  const icon7::ByteBuffer &buffer);
	static bool FileExists(std::string filePath);
};
