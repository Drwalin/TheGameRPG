#include "../../thirdparty/Collision3D/include/collision3d/CollisionShapes_AnyOrCompound.hpp"
#include "../../thirdparty/Collision3D/include/collision3d/CollisionShapes_Primitives.hpp"
#include "../../thirdparty/Collision3D/include/collision3d/CollisionShapes_HeightMap.hpp"
#include "../../thirdparty/Collision3D/include/collision3d/CollisionShapes_HeightMapHeader.hpp"
#include "../../ICon7/bitscpp/include/bitscpp/ByteReader.hpp"
#include "../../ICon7/bitscpp/include/bitscpp/ByteWriter.hpp"

#include "../../ICon7/include/icon7/ByteBuffer.hpp"
#include "../../ICon7/include/icon7/Debug.hpp"

#include "../include/GlmSerialization.hpp"

#include "../include/CollisionShapeSerialization.hpp"

namespace bitscpp
{
using namespace Collision3D;

ByteReader<true> &op(ByteReader<true> &s, Rotation &rot)
{
	s.op(rot.value);
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Rotation &rot)
{
	s.op(rot.value);
	return s;
}

ByteReader<true> &op(ByteReader<true> &s, Transform &trans)
{
	s.op(trans.pos);
	s.op(trans.rot);
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Transform &trans)
{
	s.op(trans.pos);
	s.op(trans.rot);
	return s;
}

ByteReader<true> &op(ByteReader<true> &s, VertBox &shape)
{
	s.op(shape.halfExtents);
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const VertBox &shape)
{
	s.op(shape.halfExtents);
	return s;
}

ByteReader<true> &op(ByteReader<true> &s, Cylinder &shape)
{
	s.op(shape.height);
	s.op(shape.radius);
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Cylinder &shape)
{
	s.op(shape.height);
	s.op(shape.radius);
	return s;
}

ByteReader<true> &op(ByteReader<true> &s, Sphere &shape)
{
	return s.op(shape.radius);
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Sphere &shape)
{
	return s.op(shape.radius);
}

ByteReader<true> &op(ByteReader<true> &s, RampRectangle &shape)
{
	s.op(shape.halfDepth);
	s.op(shape.halfThickness);
	s.op(shape.halfWidth);
	s.op(shape.halfHeightSkewness);
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const RampRectangle &shape)
{
	s.op(shape.halfDepth);
	s.op(shape.halfThickness);
	s.op(shape.halfWidth);
	s.op(shape.halfHeightSkewness);
	return s;
}

#define MACRO(CLASS, CODE, DEREF, SHAPE, NAME, INDEX)                          \
	case AnyPrimitive::INDEX:                                                  \
		return s.op(shape.NAME);

ByteReader<true> &op(ByteReader<true> &s, AnyPrimitive &shape)
{
	shape.~AnyPrimitive();
	s.op(*(uint8_t *)&shape.type);
	if (shape.type != AnyPrimitive::INVALID) {
		s.op(shape.pos);
		s.op(shape.rot);
		switch (shape.type) {
			EACH_PRIMITIVE(AnyPrimitive, MACRO, EMPTY_CODE);
		default:
			LOG_FATAL("Unknown collision shape id");
		}
	}
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const AnyPrimitive &shape)
{
	s.op(*(const uint8_t *)&shape.type);
	if (shape.type != AnyPrimitive::INVALID) {
		s.op(shape.pos);
		s.op(shape.rot);
		switch (shape.type) {
			EACH_PRIMITIVE(AnyPrimitive, MACRO, EMPTY_CODE);
		default:
			LOG_FATAL("Unknown collision shape id");
		}
	}
	return s;
}

#undef MACRO

ByteReader<true> &op(ByteReader<true> &s, CompoundPrimitive &shape)
{
	int32_t size;
	s.op(size);
	shape.primitives.resize(size);
	for (int i = 0; i < size; ++i) {
		s.op(shape.primitives[i]);
	}
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const CompoundPrimitive &shape)
{
	int32_t size = shape.primitives.size;
	s.op(size);
	for (int i = 0; i < size; ++i) {
		s.op(shape.primitives[i]);
	}
	return s;
}

ByteReader<true> &op(ByteReader<true> &s, HeightMap &shape)
{
	int w, h;
	s.op(w);
	s.op(h);
	if (w && h) {
		shape.Init({w, h});
		float scalex, scaley;
		s.op(scalex);
		s.op(scaley);
		shape.InitMeta(scalex, scaley);
		auto hs = shape.AccessHeights();
		for (int i = 0; i < w * h; ++i) {
			s.op(hs[i]);
		}
		auto m = shape.AccessMaterial();
		for (int i = 0; i < w * h; ++i) {
			s.op(m[i]);
		}
	}
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const HeightMap &shape)
{
	int w = 0, h = 0;
	if (shape.header == nullptr) {
		s.op(w);
		s.op(h);
		return s;
	}
	w = shape.header->resolution.x;
	h = shape.header->resolution.y;
	s.op(w);
	s.op(h);
	s.op(shape.header->scale.x);
	s.op(shape.header->scale.y);
	auto hs = shape.GetHeights();
	for (int i = 0; i < w * h; ++i) {
		s.op(hs[i]);
	}
	auto m = shape.GetMaterial();
	for (int i = 0; i < w * h; ++i) {
		s.op(m[i]);
	}
	return s;
}

#define MACRO(CLASS, CODE, DEREF, SHAPE, NAME, INDEX)                          \
	case AnyShape::INDEX:                                                      \
		return s.op(shape.NAME);

ByteReader<true> &op(ByteReader<true> &s, AnyShape &shape)
{
	shape.~AnyShape();
	s.op(*(uint8_t *)&shape.type);
	if (shape.type != AnyShape::INVALID) {
		s.op(shape.pos);
		s.op(shape.rot);
		switch (shape.type) {
		case AnyShape::INVALID:
			break;
			EACH_SHAPE(AnyPrimitive, MACRO, EMPTY_CODE);
		default:
			LOG_FATAL("Unknown collision shape id");
		}
	}
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const AnyShape &shape)
{
	s.op(*(const uint8_t *)&shape.type);
	if (shape.type != AnyShape::INVALID) {
		s.op(shape.pos);
		s.op(shape.rot);
		switch (shape.type) {
		case AnyShape::INVALID:
			break;
			EACH_SHAPE(AnyPrimitive, MACRO, EMPTY_CODE);
		default:
			LOG_FATAL("Unknown collision shape id");
		}
	}
	return s;
}

#undef MACRO

} // namespace bitscpp
