#include "../../thirdparty/glm/glm/glm.hpp"
#include "../../thirdparty/glm/glm/ext/quaternion_float.hpp"
#include "../../thirdparty/glm/glm/ext/quaternion_common.hpp"

#include <icon7/ByteBuffer.hpp>
#include <icon7/ByteReader.hpp>
#include <icon7/ByteWriter.hpp>

#include "../include/GlmSerialization.hpp"

namespace bitscpp
{
ByteReader<true> &op(ByteReader<true> &s, glm::vec3 &v)
{
	s.op(v.x);
	s.op(v.y);
	s.op(v.z);
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const glm::vec3 &v)
{
	s.op(v.x);
	s.op(v.y);
	s.op(v.z);
	return s;
}
ByteReader<true> &op(ByteReader<true> &s, glm::quat &q)
{
	s.op(q.x);
	s.op(q.y);
	s.op(q.z);
	s.op(q.w);
	return s;
}
ByteWriter<icon7::ByteBuffer> &op(ByteWriter<icon7::ByteBuffer> &s,
								  const glm::quat &q)
{
	s.op(q.x);
	s.op(q.y);
	s.op(q.z);
	s.op(q.w);
	return s;
}
} // namespace bitscpp
