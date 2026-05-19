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
	void serialize(icon7::ByteReaderBase &s);                                  \
	void serialize(icon7::ByteWriterBase &s) const;

#define BITSCPP_BYTESTREAM_OP_SYMMETRIC_DEFINITIONS(CLASS, CODE)               \
	void CLASS::serialize(icon7::ByteReaderBase &s)                            \
	{                                                                          \
		constexpr bool IS_READER = true;                                       \
		constexpr bool IS_WRITER = false;                                      \
		(void)IS_READER;                                                       \
		(void)IS_WRITER;                                                       \
		CODE;                                                                  \
	}                                                                          \
	void CLASS::serialize(icon7::ByteWriterBase &s) const                      \
	{                                                                          \
		constexpr bool IS_READER = false;                                      \
		constexpr bool IS_WRITER = true;                                       \
		(void)IS_READER;                                                       \
		(void)IS_WRITER;                                                       \
		CODE;                                                                  \
	}

#include "ForwardDeclarations.hpp" // IWYU pragma: export

#include "GlmSerialization.hpp" // IWYU pragma: export
