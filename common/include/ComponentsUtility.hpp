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
	void serialize(bitscpp::v2::ByteReader &s);  \
	void serialize(                   \
		bitscpp::v2::ByteWriter_ByteBuffer &s);

#define BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(CLASS, CODE)               \
	void CLASS::serialize(                         \
		bitscpp::v2::ByteReader &s)                                          \
	{                                                                          \
		constexpr bool IS_READER = true;                                       \
		constexpr bool IS_WRITER = false;                                      \
		(void)IS_READER;                                                       \
		(void)IS_WRITER;                                                       \
		CODE;                                                                  \
	}                                                                          \
	void CLASS::serialize(            \
		bitscpp::v2::ByteWriter_ByteBuffer &s)                             \
	{                                                                          \
		constexpr bool IS_READER = false;                                      \
		constexpr bool IS_WRITER = true;                                       \
		(void)IS_READER;                                                       \
		(void)IS_WRITER;                                                       \
		CODE;                                                                  \
	}

#include "ForwardDeclarations.hpp"

#include "GlmSerialization.hpp"
