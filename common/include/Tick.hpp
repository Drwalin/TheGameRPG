#pragma once

#include <cstdint>

#include "ComponentsUtility.hpp"

struct Tick {
	using Type = int64_t;
	Type v;

	inline constexpr bool operator< (Tick r) const { return v <  r.v; }
	inline constexpr bool operator<=(Tick r) const { return v <= r.v; }
	inline constexpr bool operator> (Tick r) const { return v >  r.v; }
	inline constexpr bool operator>=(Tick r) const { return v >= r.v; }
	inline constexpr bool operator==(Tick r) const { return v == r.v; }
	inline constexpr bool operator!=(Tick r) const { return v != r.v; }

	inline constexpr bool operator< (int64_t r) const { return v <  r; }
	inline constexpr bool operator<=(int64_t r) const { return v <= r; }
	inline constexpr bool operator> (int64_t r) const { return v >  r; }
	inline constexpr bool operator>=(int64_t r) const { return v >= r; }
	inline constexpr bool operator==(int64_t r) const { return v == r; }
	inline constexpr bool operator!=(int64_t r) const { return v != r; }

	
	inline constexpr Tick &operator++()    { ++v; return *this; }
	inline constexpr Tick &operator--()    { --v; return *this; }
	inline constexpr Tick  operator++(int) { return {v++}; }
	inline constexpr Tick  operator--(int) { return {v--}; }

	inline constexpr Tick operator-() const { return {-v}; }
	inline constexpr Tick operator+() const { return {v}; }

	
	inline constexpr Tick operator-(Tick b) const { return {v - b.v}; }
	inline constexpr Tick operator+(Tick b) const { return {v + b.v}; }
	
	inline constexpr Tick operator+(int r)     const { return {v + r}; }
	inline constexpr Tick operator-(int r)     const { return {v - r}; }
	inline constexpr Tick operator+(int64_t r) const { return {v + r}; }
	inline constexpr Tick operator-(int64_t r) const { return {v - r}; }
	inline constexpr Tick operator+(float r)   const { return {Type(v + r)}; }
	inline constexpr Tick operator-(float r)   const { return {Type(v - r)}; }
	inline constexpr Tick operator+(double r)  const { return {Type(v + r)}; }
	inline constexpr Tick operator-(double r)  const { return {Type(v - r)}; }

	inline constexpr Tick operator*(int r)     const { return {v * r}; }
	inline constexpr Tick operator/(int r)     const { return {v / r}; }
	inline constexpr Tick operator*(int64_t r) const { return {v * r}; }
	inline constexpr Tick operator/(int64_t r) const { return {v / r}; }
	inline constexpr Tick operator*(float r)   const { return {Type(v * r)}; }
	inline constexpr Tick operator/(float r)   const { return {Type(v / r)}; }
	inline constexpr Tick operator*(double r)  const { return {Type(v * r)}; }
	inline constexpr Tick operator/(double r)  const { return {Type(v / r)}; }

	
	inline constexpr Tick &operator+=(Tick b) { v += b.v; return *this; }
	inline constexpr Tick &operator-=(Tick b) { v -= b.v; return *this; }
	
	inline constexpr Tick &operator+=(int r)     { v += r; return *this; }
	inline constexpr Tick &operator-=(int r)     { v -= r; return *this; }
	inline constexpr Tick &operator+=(int64_t r) { v += r; return *this; }
	inline constexpr Tick &operator-=(int64_t r) { v -= r; return *this; }
	inline constexpr Tick &operator+=(float r)   { v += r; return *this; }
	inline constexpr Tick &operator-=(float r)   { v -= r; return *this; }
	inline constexpr Tick &operator+=(double r)  { v += r; return *this; }
	inline constexpr Tick &operator-=(double r)  { v -= r; return *this; }

	inline constexpr Tick &operator*=(int r)     { v *= r; return *this; }
	inline constexpr Tick &operator/=(int r)     { v /= r; return *this; }
	inline constexpr Tick &operator*=(int64_t r) { v *= r; return *this; }
	inline constexpr Tick &operator/=(int64_t r) { v /= r; return *this; }
	inline constexpr Tick &operator*=(float r)   { v *= r; return *this; }
	inline constexpr Tick &operator/=(float r)   { v /= r; return *this; }
	inline constexpr Tick &operator*=(double r)  { v *= r; return *this; }
	inline constexpr Tick &operator/=(double r)  { v /= r; return *this; }
	
	BITSCPP_BYTESTREAM_OP_DECLARATIONS()
};
