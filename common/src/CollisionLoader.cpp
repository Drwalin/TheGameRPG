#include <fstream>

#include <icon7/Debug.hpp>

#include "../include/CollisionLoader.hpp"

void CollisionLoader::LoadOBJ(const void *fileBuffer, size_t bytes)
{
	collisionData.indices.reserve(1000);
	collisionData.vertices.reserve(1000);
	collisionData.indices.clear();
	collisionData.vertices.clear();
	std::vector<uint32_t> ind;
	ind.reserve(16);
	char line[4096];
	char word[1024];
	glm::vec3 v;

	char const *begin = (const char *)fileBuffer;
	char const *end = (const char *)fileBuffer + bytes;
	char const *ptr = begin;

	while (ptr + 2 < end) {
		const char *endLine = (const char *)memchr(ptr + 1, '\n', end - ptr);

		size_t count = endLine - ptr;
		if (count > 4094) {
			count = 4094;
			LOG_FATAL("Line in OBJ file longer than maximum 4094 bytes");
		}
		memcpy(line, ptr, count);
		line[count] = 0;
		ptr = endLine;

		char *lineOffset = line;
		int off = 0;
		sscanf(lineOffset, "%s%n", word, &off);
		lineOffset += off;

		switch (word[0]) {
		case 'v':
			if (word[1] == 0) {
				sscanf(lineOffset, "%f %f %f", &v.x, &v.y, &v.z);
				collisionData.vertices.push_back(v);
			}
			break;
		case 'f':
			ind.clear();
			while (true) {
				off = 0;
				word[0] = 0;
				if (sscanf(lineOffset, "%s%n", word, &off) == 0) {
					break;
				}
				lineOffset += off;
				if (word[0] == 0)
					break;
				ind.push_back(atoi(word) - 1);
			}
			for (uint32_t j = 0; j + 2 < ind.size(); ++j) {
				collisionData.indices.push_back(ind[0]);
				collisionData.indices.push_back(ind[j + 1]);
				collisionData.indices.push_back(ind[j + 2]);
			}
			break;
		}
	}
	collisionData.indices.shrink_to_fit();
	collisionData.vertices.shrink_to_fit();
}

bool CollisionLoader::LoadOBJ(std::string fileName)
{
	std::ifstream stream(fileName);
	if (stream.good() == false) {
		LOG_FATAL("Failed to open file: `%s`", fileName.c_str());
		return false;
	}

	stream.seekg(0, std::ios::end);
	std::ifstream::pos_type fileSize = stream.tellg();
	stream.seekg(0, std::ios::beg);

	char *buffer = (char *)malloc(fileSize);
	stream.read(buffer, fileSize);

	LoadOBJ(buffer, fileSize);
	return true;
}
