#pragma once

#include "ForwardDeclarations.hpp"

#include "../../thirdparty/Collision3D/include/collision3d/ForwardDeclarations.hpp"

namespace bitscpp
{
ByteReader<true> &op(ByteReader<true> &s, Collision3D::Rotation &rot);
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::Rotation &rot);

ByteReader<true> &op(ByteReader<true> &s, Collision3D::Transform &trans);
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::Transform &trans);

ByteReader<true> &op(ByteReader<true> &s, Collision3D::VertBox &shape);
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::VertBox &shape);

ByteReader<true> &op(ByteReader<true> &s, Collision3D::Cylinder &shape);
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::Cylinder &shape);

ByteReader<true> &op(ByteReader<true> &s, Collision3D::Sphere &shape);
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::Sphere &shape);

ByteReader<true> &op(ByteReader<true> &s, Collision3D::RampRectangle &shape);
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::RampRectangle &shape);

ByteReader<true> &op(ByteReader<true> &s, Collision3D::AnyPrimitive &shape);
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::AnyPrimitive &shape);

ByteReader<true> &op(ByteReader<true> &s,
					 Collision3D::CompoundPrimitive &shape);
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::CompoundPrimitive &shape);

ByteReader<true> &op(ByteReader<true> &s, Collision3D::HeightMap &shape);
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::HeightMap &shape);

ByteReader<true> &op(ByteReader<true> &s, Collision3D::AnyShape &shape);
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::AnyShape &shape);
} // namespace bitscpp
