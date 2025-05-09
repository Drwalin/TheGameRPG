#pragma once

#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/glm.hpp"
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/quaternion_float.hpp"
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/quaternion_common.hpp"

#include "ComponentsUtility.hpp"

namespace bitscpp
{
ByteReader<true> &op(ByteReader<true> &s, glm::vec3 &v);

ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const glm::vec3 &v);

ByteReader<true> &op(ByteReader<true> &s, glm::quat &q);

ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const glm::quat &q);
} // namespace bitscpp
