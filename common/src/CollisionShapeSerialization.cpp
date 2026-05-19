#include "../../thirdparty/Collision3D/include/collision3d/CollisionShapes_AnyOrCompound.hpp"
#include "../../thirdparty/Collision3D/include/collision3d/CollisionShapes_Primitives.hpp"
#include "../../thirdparty/Collision3D/include/collision3d/CollisionShapes_HeightMap.hpp"
#include "../../thirdparty/Collision3D/include/collision3d/CollisionShapes_HeightMapHeader.hpp"

#include "../../ICon7/include/icon7/ByteReader.hpp" // IWYU pragma: export
#include "../../ICon7/include/icon7/ByteWriter.hpp" // IWYU pragma: export
#include "../../ICon7/include/icon7/Debug.hpp"

#include "../include/GlmSerialization.hpp"

#include "../include/CollisionShapeSerialization.hpp"

using namespace Collision3D;

namespace bitscpp
{
namespace v2
{
void serialize(icon7::ByteReaderBase &s, Rotation &rot) { s.op(rot.value); }
void serialize(icon7::ByteWriterBase &s, const Rotation &rot)
{
	s.op(rot.value);
}

void serialize(icon7::ByteReaderBase &s, Transform &trans)
{
	s.op(trans.pos);
	s.op(trans.rot);
}
void serialize(icon7::ByteWriterBase &s, const Transform &trans)
{
	s.op(trans.pos);
	s.op(trans.rot);
}

void serialize(icon7::ByteReaderBase &s, VertBox &shape)
{
	s.op(shape.halfExtents);
}
void serialize(icon7::ByteWriterBase &s, const VertBox &shape)
{
	s.op(shape.halfExtents);
}

void serialize(icon7::ByteReaderBase &s, Cylinder &shape)
{
	s.op(shape.height);
	s.op(shape.radius);
}
void serialize(icon7::ByteWriterBase &s, const Cylinder &shape)
{
	s.op(shape.height);
	s.op(shape.radius);
}

void serialize(icon7::ByteReaderBase &s, Sphere &shape)
{
	s.op(shape.radius);
}
void serialize(icon7::ByteWriterBase &s, const Sphere &shape)
{
	s.op(shape.radius);
}

void serialize(icon7::ByteReaderBase &s, RampRectangle &shape)
{
	s.op(shape.halfDepth);
	s.op(shape.halfThickness);
	s.op(shape.halfWidth);
	s.op(shape.halfHeightSkewness);
}
void serialize(icon7::ByteWriterBase &s,
			   const RampRectangle &shape)
{
	s.op(shape.halfDepth);
	s.op(shape.halfThickness);
	s.op(shape.halfWidth);
	s.op(shape.halfHeightSkewness);
}

#define MACRO(CLASS, CODE, DEREF, SHAPE, NAME, INDEX)                          \
	case AnyPrimitive::INDEX:                                                  \
		s.op(shape.NAME);                                                      \
		return;

void serialize(icon7::ByteReaderBase &s, AnyPrimitive &shape)
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
}
void serialize(icon7::ByteWriterBase &s, const AnyPrimitive &shape)
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
}

#undef MACRO

void serialize(icon7::ByteReaderBase &s, CompoundPrimitive &shape)
{
	int32_t size;
	s.op(size);
	shape.primitives.resize(size);
	for (int i = 0; i < size; ++i) {
		s.op(shape.primitives[i]);
	}
}
void serialize(icon7::ByteWriterBase &s,
			   const CompoundPrimitive &shape)
{
	int32_t size = shape.primitives.size;
	s.op(size);
	for (int i = 0; i < size; ++i) {
		s.op(shape.primitives[i]);
	}
}

void serialize(icon7::ByteReaderBase &s, HeightMap &shape)
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
}
void serialize(icon7::ByteWriterBase &s, const HeightMap &shape)
{
	int w = 0, h = 0;
	if (shape.header == nullptr) {
		s.op(w);
		s.op(h);
		return;
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
}

#define MACRO(CLASS, CODE, DEREF, SHAPE, NAME, INDEX)                          \
	case AnyShape::INDEX:                                                      \
		s.op(shape.NAME);                                                      \
		return;

void serialize(icon7::ByteReaderBase &s, AnyShape &shape)
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
}
void serialize(icon7::ByteWriterBase &s, const AnyShape &shape)
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
}

#undef MACRO

} // namespace v2
} // namespace bitscpp
