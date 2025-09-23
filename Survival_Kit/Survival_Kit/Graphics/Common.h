#pragma once

#ifndef __COMMON_H__
#define __COMMON_H__
#include "../Glad/glad.h"
#include <cstdint>

namespace gam300 {

	using u8  = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;

	using i8  = int8_t;
	using i16 = int16_t;
	using i32 = int32_t;
	using i64 = int64_t;

	static_assert(sizeof(GLuint) == sizeof(u32), "GLuint mismatch");
	static_assert(sizeof(GLubyte) == sizeof(u8), "GLubyte mismatch");
}

#endif // !__COMMON_H__
