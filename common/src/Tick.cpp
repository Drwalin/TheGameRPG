#include "../../ICon7/include/icon7/ByteBuffer.hpp"
#include "../../ICon7/include/icon7/ByteReader.hpp"
#include "../../ICon7/include/icon7/ByteWriter.hpp"

#include "../include/Tick.hpp"

BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(Tick, {
	s.op(v);
});
