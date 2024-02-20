#include <random>

#include <glm/geometric.hpp>

#include <TerrainMap.hpp>

void TerrainMap::Init(uint32_t width, uint32_t depth, float horizontalScale)
{
	this->width = width;
	this->depth = depth;
	this->horizontalScale = horizontalScale;
	horizontalScaleInverse = 1.0f/horizontalScale;
	heights.resize(width*depth);
}

void TerrainMap::Generate(int octaves, float lacunarity, float maxHeight)
{
	PerlinGenerator generator;
	generator.Generate(this, width, depth, 5, 0.3f, 100.0f);
}

bool TerrainMap::GetHeight(glm::vec3 pos, float *height) const
{
	return GetHeightAndNormal(pos, height, nullptr);
}

bool TerrainMap::GetHeightAndNormal(glm::vec3 pos, float *height,
						glm::vec3 *normal) const
{
	if (normal == nullptr) {
		return GetHeightAndDeltas(pos, height, nullptr, nullptr);
	}
	
	float dx, dy;
	if (GetHeightAndDeltas(pos, height, &dx, &dy) == false) {
		return false;
	}
	
	glm::vec3 vx(0, dx, 1);
	glm::vec3 vy(1, dy, 0);
	
	glm::vec3 &n = *normal;
	
	n.x = /*vx.y*vy.z*/ - vx.z*vy.y;
	n.y = vx.z*vy.x /*- vx.x*vy.z*/;
	n.z = /*vx.x*vy.y*/ - vx.y*vy.x;
	
	n = glm::normalize(n);
	return true;
}

bool TerrainMap::GetHeightAndDeltas(glm::vec3 pos, float *height,
						float *dx, float *dy) const
{
	pos *= horizontalScaleInverse;
	if (pos.x < 0 || pos.z < 0 || pos.x >= width-2.00001 || pos.z >= depth-2.00001) {
		return false;
	}
	
	int x = pos.x;
	int y = pos.y;
	
	float hs[2][2];
	hs[0][0] = GetHeight(x, y);
	hs[0][1] = GetHeight(x, y+1);
	hs[1][0] = GetHeight(x+1, y);
	hs[1][1] = GetHeight(x+1, y+1);
	
	float fx = pos.x - x;
	float fy = pos.y - y;
	
	float dx1 = hs[0][0] * (1-fx) + hs[1][0] * fx;
	float dx2 = hs[0][1] * (1-fx) + hs[1][1] * fx;
	
	if (height) {
		*height = dx1 * (1-fy) + dx2 * fy;
	}
	
	if (dx && dy) {
		float dy1 = hs[0][0] * (1-fy) + hs[0][1] * fy;
		float dy2 = hs[1][0] * (1-fy) + hs[1][1] * fy;
		
		*dx = dx2-dx1;
		*dy = dy2-dy1;
	}
	return true;
}


void TerrainMap::SetHeight(int x, int z, float height)
{
	if (x < 0 || z < 0 || x >= width-1 || z >= depth-1) {
		return;
	}
	heights.data()[x+(z*width)] = height;
}

void PerlinGenerator::Generate(TerrainMap *dstMap, uint32_t width,
							   uint32_t depth, int octaves, float lacunarity,
							   float maxHeight)
{
	std::random_device rd;
	std::mt19937_64 mt(rd());
	
	float totalMultiplier = 1, mult = 1;
	for (int i=1; i<octaves; ++i) {
		mult *= lacunarity;
		totalMultiplier += mult;
	}
	
	float currentMaxHeight = maxHeight / totalMultiplier;
	
	TerrainMap m[2];
	TerrainMap *next = m, *prev = m+1;
	
	uint32_t maxDimension = std::max(width, depth);
	uint32_t currentSize = maxDimension;
	
	for (int i=0; i<octaves; ++i) {
		currentSize = (currentSize+1)>>1;
	}
	
	prev->Init(currentSize, currentSize, 2);
	
	std::uniform_real_distribution<float> heightDistribution(0.0f, 1.0f);
	for (float &h : prev->heights) {
		h = heightDistribution(mt) * currentMaxHeight;
	}
	
	for (; currentSize<maxDimension;) {
		currentMaxHeight *= lacunarity;
		
		currentSize = (currentSize<<1)-1;
		
		next->Init(currentSize, currentSize, 2);
		
		if (currentSize >= maxDimension) {
			next = dstMap;
		}
		
		for (int x=0; x<next->width-1; ++x) {
			for (int y=0; y<next->depth-1; ++y) {
				float h = 0;
				prev->GetHeight({x, 0, y}, &h);
				h += heightDistribution(mt) * currentMaxHeight;
				next->SetHeight(x, y, h);
			}
		}
		
		std::swap(next, prev);
	}
}
