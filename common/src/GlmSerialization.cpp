#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/glm.hpp" // IWYU pragma: keep
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/quaternion_float.hpp" // IWYU pragma: keep
#include "../../thirdparty/Collision3D/SpatialPartitioning/glm/glm/ext/quaternion_common.hpp" // IWYU pragma: keep

#include "../../ICon7/include/icon7/ByteReader.hpp"
#include "../../ICon7/include/icon7/ByteWriter.hpp"

#include "../include/GlmSerialization.hpp"

namespace bitscpp
{
namespace v2
{
void serialize(icon7::ByteReaderBase &s, glm::vec3 &v)
{
	s.op(v.x);
	s.op(v.y);
	s.op(v.z);
}
void serialize(icon7::ByteWriterBase &s, const glm::vec3 &v)
{
	s.op(v.x);
	s.op(v.y);
	s.op(v.z);
}
void serialize(icon7::ByteReaderBase &s, glm::quat &q)
{
	s.op(q.x);
	s.op(q.y);
	s.op(q.z);
	s.op(q.w);
}
void serialize(icon7::ByteWriterBase &s, const glm::quat &q)
{
	s.op(q.x);
	s.op(q.y);
	s.op(q.z);
	s.op(q.w);
}
} // namespace v2
} // namespace bitscpp
