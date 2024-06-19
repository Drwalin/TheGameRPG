#pragma once

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_common.hpp>

#include "../../ICon7/bitscpp/include/bitscpp/ByteReader.hpp"
#include "../../ICon7/bitscpp/include/bitscpp/ByteWriter.hpp"

namespace bitscpp
{
template <>
inline ByteReader<true> &op<glm::vec3>(ByteReader<true> &s, glm::vec3 &v)
{
	s.op(v.x);
	s.op(v.y);
	s.op(v.z);
	return s;
}
template <typename BT>
inline ByteWriter<BT> &op(ByteWriter<BT> &s, const glm::vec3 &v)
{
	s.op(v.x);
	s.op(v.y);
	s.op(v.z);
	return s;
}
template <>
inline ByteReader<true> &op<glm::quat>(ByteReader<true> &s, glm::quat &q)
{
	s.op(q.x);
	s.op(q.y);
	s.op(q.z);
	s.op(q.w);
	return s;
}
template <typename BT>
inline ByteWriter<BT> &op(ByteWriter<BT> &s, const glm::quat &q)
{
	s.op(q.x);
	s.op(q.y);
	s.op(q.z);
	s.op(q.w);
	return s;
}
} // namespace bitscpp
