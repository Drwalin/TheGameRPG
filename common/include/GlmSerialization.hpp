#pragma once

#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/glm.hpp" // IWYU pragma: export
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/quaternion_float.hpp" // IWYU pragma: export
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/quaternion_common.hpp" // IWYU pragma: export

#include "ComponentsUtility.hpp" // IWYU pragma: export

namespace bitscpp
{
namespace v2
{
void serialize(icon7::ByteReaderBase &s, glm::vec3 &v);
void serialize(icon7::ByteWriterBase &s, const glm::vec3 &v);

void serialize(icon7::ByteReaderBase &s, glm::quat &q);
void serialize(icon7::ByteWriterBase &s, const glm::quat &q);
} // namespace v2
} // namespace bitscpp
