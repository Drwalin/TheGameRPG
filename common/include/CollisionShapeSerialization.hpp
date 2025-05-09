#pragma once

#include "ComponentsUtility.hpp"

#include "../../thirdparty/Collision3D/include/collision3d/ForwardDeclarations.hpp"

namespace bitscpp {
	ByteReader<true>& op(ByteReader<true>& s, Collision3D::VertBox& shape);
	ByteWriter<icon7::ByteBuffer>& op(ByteWriter<icon7::ByteBuffer>& s, const Collision3D::VertBox& shape);
	
	ByteReader<true>& op(ByteReader<true>& s, Collision3D::Cylinder& shape);
	ByteWriter<icon7::ByteBuffer>& op(ByteWriter<icon7::ByteBuffer>& s, const Collision3D::Cylinder& shape);
	
	ByteReader<true>& op(ByteReader<true>& s, Collision3D::HeightMap<float>& shape);
	ByteWriter<icon7::ByteBuffer>& op(ByteWriter<icon7::ByteBuffer>& s, const Collision3D::HeightMap<float>& shape);
}
