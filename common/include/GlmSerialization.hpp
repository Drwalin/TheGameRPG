#pragma once

#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/glm.hpp"
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/quaternion_float.hpp"
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/quaternion_common.hpp"

#include "ComponentsUtility.hpp"

namespace bitscpp
{
namespace v2
{
void serialize(bitscpp::v2::ByteReader &s, glm::vec3 &v);
void serialize(bitscpp::v2::ByteWriter_ByteBuffer &s, const glm::vec3 &v);

void serialize(bitscpp::v2::ByteReader &s, glm::quat &q);
void serialize(bitscpp::v2::ByteWriter_ByteBuffer &s, const glm::quat &q);
} // namespace v2
} // namespace bitscpp
