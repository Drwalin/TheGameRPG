#pragma once

#include <glm/glm.hpp>

#include <bitscpp/ByteReader.hpp>
#include <bitscpp/ByteWriter.hpp>

namespace bitscpp {
	template<>
	inline ByteReader<true>& op<glm::vec3>(ByteReader<true>& s, glm::vec3& v) {
		s.op(v.x);
		s.op(v.y);
		s.op(v.z);
		return s;
	}
	template<>
	inline ByteWriter& op<glm::vec3>(ByteWriter& s, const glm::vec3& v) {
		s.op(v.x);
		s.op(v.y);
		s.op(v.z);
		return s;
	}
}

