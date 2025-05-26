#pragma once

#define MV(VAR) this->VAR = o.VAR;

#define DEFAULT_CONSTRUCTORS_AND_MOVE(STRUCT, CODE)                            \
	inline STRUCT(const STRUCT &o) { CODE }                                    \
	inline STRUCT(STRUCT &o) { CODE }                                          \
	inline STRUCT(STRUCT &&o) { CODE }                                         \
	inline STRUCT() {}                                                         \
	inline ~STRUCT() {}                                                        \
	inline STRUCT &operator=(const STRUCT &o) { CODE return *this; }           \
	inline STRUCT &operator=(STRUCT &o) { CODE return *this; }                 \
	inline STRUCT &operator=(STRUCT &&o) { CODE return *this; }

#define BITSCPP_BYTESTREAM_OP_DECLARATIONS()                                   \
	bitscpp::ByteReader<true> &__ByteStream_op(bitscpp::ByteReader<true> &s);  \
	bitscpp::ByteWriter<icon7::ByteBuffer> &__ByteStream_op(                   \
		bitscpp::ByteWriter<icon7::ByteBuffer> &s)

#define BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(CLASS, CODE)               \
	bitscpp::ByteReader<true> &CLASS::__ByteStream_op(                         \
		bitscpp::ByteReader<true> &s)                                          \
	{                                                                          \
		CODE;                                                                  \
		return s;                                                              \
	}                                                                          \
	bitscpp::ByteWriter<icon7::ByteBuffer> &CLASS::__ByteStream_op(            \
		bitscpp::ByteWriter<icon7::ByteBuffer> &s)                             \
	{                                                                          \
		CODE;                                                                  \
		return s;                                                              \
	}

#include "ForwardDeclarations.hpp"

#include "GlmSerialization.hpp"
