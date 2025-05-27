#pragma once

#include <cstdint>

#include "ForwardDeclarations.hpp"

#include "../../thirdparty/Collision3D/include/collision3d/ForwardDeclarations.hpp"

namespace bitscpp
{
ByteReader<true> &op(ByteReader<true> &s, Collision3D::VertBox &shape);
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::VertBox &shape);

ByteReader<true> &op(ByteReader<true> &s, Collision3D::Cylinder &shape);
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::Cylinder &shape);

ByteReader<true> &op(ByteReader<true> &s,
					 Collision3D::HeightMap<float, uint8_t> &shape);
ByteWriter<icon7::ByteBuffer> &
op(ByteWriter<icon7::ByteBuffer> &s,
   const Collision3D::HeightMap<float, uint8_t> &shape);

ByteReader<true> &op(ByteReader<true> &s, Collision3D::Rotation &rot);
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::Rotation &rot);

ByteReader<true> &op(ByteReader<true> &s, Collision3D::Transform &trans);
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const Collision3D::Transform &trans);
} // namespace bitscpp
