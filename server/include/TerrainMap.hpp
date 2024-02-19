#pragma once

#include <cstdio>
#include <cinttypes>

#include <typeinfo>
#include <type_traits>
#include <vector>

#include <glm/glm.hpp>

#include <bitscpp/ByteReader.hpp>

class TerrainMap
{
public:
	TerrainMap();

	void Init(uint32_t width, uint32_t depth, float horizontalScale);
	void Generate(int octaves, float lacunarity, float maxHeight);

	bool GetHeight(glm::vec3 pos, float *height) const;
	bool GetHeightAndNormal(glm::vec3 pos, float *height,
							glm::vec3 *normal) const;
	bool GetHeightAndDeltas(glm::vec3 pos, float *height,
							float *dx, float *dy) const;
	
	void SetHeight(int x, int z, float height);
	
public:

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(width);
		s.op(depth);
		s.op(horizontalScale);
		if constexpr (std::is_same_v<S, bitscpp::ByteReader<true>> ||
					  std::is_same_v<S, bitscpp::ByteReader<false>>) {
			heights.resize(width * depth);
		}
		s.op(heights.data(), width * depth);
		return s;
	}
	
private:
	float GetHeight(int x, int z) const;

	friend class PerlinGenerator;
	
private:
	uint32_t width, depth;
	std::vector<float> heights;
	float horizontalScale;
	float horizontalScaleInverse;
};

class PerlinGenerator
{
public:
	
	void Generate(TerrainMap *dstMap, uint32_t width, uint32_t depth,
				  int octaves, float lacunarity, float maxHeight);
};
