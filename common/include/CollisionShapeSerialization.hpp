#pragma once

#include "ForwardDeclarations.hpp"

#include "../../thirdparty/Collision3D/include/collision3d/ForwardDeclarations.hpp"

namespace bitscpp
{
namespace v2
{
void serialize(bitscpp::v2::ByteReader &s, Collision3D::Rotation &rot);
void serialize(bitscpp::v2::ByteWriter_ByteBuffer &s,
			   const Collision3D::Rotation &rot);

void serialize(bitscpp::v2::ByteReader &s, Collision3D::Transform &trans);
void serialize(bitscpp::v2::ByteWriter_ByteBuffer &s,
			   const Collision3D::Transform &trans);

void serialize(bitscpp::v2::ByteReader &s, Collision3D::VertBox &shape);
void serialize(bitscpp::v2::ByteWriter_ByteBuffer &s,
			   const Collision3D::VertBox &shape);

void serialize(bitscpp::v2::ByteReader &s, Collision3D::Cylinder &shape);
void serialize(bitscpp::v2::ByteWriter_ByteBuffer &s,
			   const Collision3D::Cylinder &shape);

void serialize(bitscpp::v2::ByteReader &s, Collision3D::Sphere &shape);
void serialize(bitscpp::v2::ByteWriter_ByteBuffer &s,
			   const Collision3D::Sphere &shape);

void serialize(bitscpp::v2::ByteReader &s, Collision3D::RampRectangle &shape);
void serialize(bitscpp::v2::ByteWriter_ByteBuffer &s,
			   const Collision3D::RampRectangle &shape);

void serialize(bitscpp::v2::ByteReader &s, Collision3D::AnyPrimitive &shape);
void serialize(bitscpp::v2::ByteWriter_ByteBuffer &s,
			   const Collision3D::AnyPrimitive &shape);

void serialize(bitscpp::v2::ByteReader &s,
			   Collision3D::CompoundPrimitive &shape);
void serialize(bitscpp::v2::ByteWriter_ByteBuffer &s,
			   const Collision3D::CompoundPrimitive &shape);

void serialize(bitscpp::v2::ByteReader &s, Collision3D::HeightMap &shape);
void serialize(bitscpp::v2::ByteWriter_ByteBuffer &s,
			   const Collision3D::HeightMap &shape);

void serialize(bitscpp::v2::ByteReader &s, Collision3D::AnyShape &shape);
void serialize(bitscpp::v2::ByteWriter_ByteBuffer &s,
			   const Collision3D::AnyShape &shape);
} // namespace v2
} // namespace bitscpp
