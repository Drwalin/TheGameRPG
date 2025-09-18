#include "../../thirdparty/Collision3D/include/collision3d/CollisionShapes.hpp"
#include "../../ICon7/bitscpp/include/bitscpp/ByteReader.hpp"
#include "../../ICon7/bitscpp/include/bitscpp/ByteWriter.hpp"

#include "../../ICon7/include/icon7/ByteBuffer.hpp"
#include "../../ICon7/include/icon7/Debug.hpp"

#include "../include/GlmSerialization.hpp"
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

ByteReader<true> &op(ByteReader<true> &s,
					 Collision3D::HeightMap<float, uint8_t> &shape)
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
ByteWriter<icon7::ByteBuffer> &
op(ByteWriter<icon7::ByteBuffer> &s,
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





ByteReader<true> &op(ByteReader<true> &s, Collision3D::Sphere &shape)
{
	return s.op(shape.radius);
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::Sphere &shape)
{
	return s.op(shape.radius);
}

ByteReader<true> &op(ByteReader<true> &s, Collision3D::Rectangle &shape)
{
	s.op(shape.depth);
	s.op(shape.width);
	return s.op(shape.height);
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::Rectangle &shape)
{
	s.op(shape.depth);
	s.op(shape.width);
	return s.op(shape.height);
}

ByteReader<true> &op(ByteReader<true> &s, Collision3D::AnyPrimitive &shape)
{
	shape.~AnyPrimitive();
	s.op(*(uint8_t*)&shape.type);
	if (shape.type != Collision3D::AnyPrimitive::INVALID) {
		s.op(shape.trans);
	}
	switch (shape.type) {
	case Collision3D::AnyPrimitive::INVALID:
		break;
	case Collision3D::AnyPrimitive::VERTBOX:
		return s.op(shape.vertBox);
	case Collision3D::AnyPrimitive::CYLINDER:
		return s.op(shape.cylinder);
	case Collision3D::AnyPrimitive::SPHERE:
		return s.op(shape.sphere);
	case Collision3D::AnyPrimitive::RECTANGLE:
		return s.op(shape.rectangle);
	case Collision3D::AnyPrimitive::VERTICAL_TRIANGLE:
	case Collision3D::AnyPrimitive::CAPPED_CONE:
		LOG_FATAL("Collision shapes not implemented");
		break;
	default:
		LOG_FATAL("Unknown collision shape id");
	}
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::AnyPrimitive &shape)
{
	s.op(*(const uint8_t*)&shape.type);
	if (shape.type != Collision3D::AnyPrimitive::INVALID) {
		s.op(shape.trans);
	}
	switch (shape.type) {
	case Collision3D::AnyPrimitive::INVALID:
		break;
	case Collision3D::AnyPrimitive::VERTBOX:
		return s.op(shape.vertBox);
	case Collision3D::AnyPrimitive::CYLINDER:
		return s.op(shape.cylinder);
	case Collision3D::AnyPrimitive::SPHERE:
		return s.op(shape.sphere);
	case Collision3D::AnyPrimitive::RECTANGLE:
		return s.op(shape.rectangle);
	case Collision3D::AnyPrimitive::VERTICAL_TRIANGLE:
	case Collision3D::AnyPrimitive::CAPPED_CONE:
		LOG_FATAL("Collision shapes not implemented");
		break;
	default:
		LOG_FATAL("Unknown collision shape id");
	}
	return s;
}

ByteReader<true> &op(ByteReader<true> &s, Collision3D::CompoundPrimitive &shape)
{
	return s.op(shape.primitives);
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::CompoundPrimitive &shape)
{
	return s.op(shape.primitives);
}

ByteReader<true> &op(ByteReader<true> &s, Collision3D::AnyShape &shape)
{
	shape.~AnyShape();
	s.op(*(uint8_t*)&shape.type);
	if (shape.type != Collision3D::AnyShape::INVALID) {
		s.op(shape.trans);
	}
	
	switch(shape.type) {
	case Collision3D::AnyShape::INVALID:
		break;
	case Collision3D::AnyShape::VERTBOX:
		return s.op(shape.vertBox);
	case Collision3D::AnyShape::CYLINDER:
		return s.op(shape.cylinder);
	case Collision3D::AnyShape::SPHERE:
		return s.op(shape.sphere);
	case Collision3D::AnyShape::RECTANGLE:
		return s.op(shape.rectangle);
	case Collision3D::AnyShape::VERTICAL_TRIANGLE:
	case Collision3D::AnyShape::CAPPED_CONE:
		LOG_FATAL("Collision shapes not implemented");
		break;
	case Collision3D::AnyShape::HEIGHT_MAP:
		shape.heightMap = new Collision3D::HeightMap<float, uint8_t>;
		return s.op(*shape.heightMap);
	case Collision3D::AnyShape::COMPOUND:
		return s.op(shape.compound);
	default:
		LOG_FATAL("Unknown collision shape id");
	}
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::AnyShape &shape)
{
	s.op(*(const uint8_t*)&shape.type);
	if (shape.type != Collision3D::AnyShape::INVALID) {
		s.op(shape.trans);
	}
	switch(shape.type) {
	case Collision3D::AnyShape::INVALID:
		break;
	case Collision3D::AnyShape::VERTBOX:
		return s.op(shape.vertBox);
	case Collision3D::AnyShape::CYLINDER:
		return s.op(shape.cylinder);
	case Collision3D::AnyShape::SPHERE:
		return s.op(shape.sphere);
	case Collision3D::AnyShape::RECTANGLE:
		return s.op(shape.rectangle);
	case Collision3D::AnyShape::VERTICAL_TRIANGLE:
	case Collision3D::AnyShape::CAPPED_CONE:
		LOG_FATAL("Collision shapes not implemented");
		break;
	case Collision3D::AnyShape::HEIGHT_MAP:
		return s.op(*shape.heightMap);
	case Collision3D::AnyShape::COMPOUND:
		return s.op(shape.compound);
	default:
		LOG_FATAL("Unknown collision shape id");
	}
	return s;
}





ByteReader<true> &op(ByteReader<true> &s, Collision3D::Rotation &rot)
{
	s.op(rot.value);
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::Rotation &rot)
{
	s.op(rot.value);
	return s;
}

ByteReader<true> &op(ByteReader<true> &s, Collision3D::Transform &trans)
{
	s.op(trans.pos);
	s.op(trans.rot);
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::Transform &trans)
{
	s.op(trans.pos);
	s.op(trans.rot);
	return s;
}

} // namespace bitscpp
