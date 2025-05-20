#include "../../thirdparty/Collision3D/include/collision3d/CollisionShapes.hpp"
#include "../../ICon7/bitscpp/include/bitscpp/ByteReader.hpp"
#include "../../ICon7/bitscpp/include/bitscpp/ByteWriter.hpp"

#include "../../ICon7/include/icon7/ByteBuffer.hpp"

#include "../include/CollisionShapeSerialization.hpp"

namespace bitscpp
{
ByteReader<true> &op(ByteReader<true> &s, Collision3D::VertBox &shape)
{
	s.op(shape.halfExtents);
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::VertBox &shape)
{
	s.op(shape.halfExtents);
	return s;
}

ByteReader<true> &op(ByteReader<true> &s, Collision3D::Cylinder &shape)
{
	s.op(shape.height);
	s.op(shape.radius);
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::Cylinder &shape)
{
	s.op(shape.height);
	s.op(shape.radius);
	return s;
}

ByteReader<true> &op(ByteReader<true> &s, Collision3D::HeightMap<float, uint8_t> &shape)
{
	glm::vec3 size, scale;
	int width, height;
	s.op(size);
	s.op(scale);
	s.op(width);
	s.op(height);
	shape.InitValues(width, height, scale, size);
	s.op(shape.mipmap[0].heights);
	s.op(shape.material.heights);
	shape.GenerateMipmap();
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::HeightMap<float, uint8_t> &shape)
{
	s.op(shape.size);
	s.op(shape.scale);
	s.op(shape.width);
	s.op(shape.height);
	s.op(shape.mipmap[0].heights);
	s.op(shape.material.heights);
	return s;
}

} // namespace bitscpp
