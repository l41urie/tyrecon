#pragma once
#include <cstdint>

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

#define ASSERT(cond) if(!(cond)) __debugbreak();

#define DBG_PAUSE(msg...) { printf(msg); system("pause"); }

#define PURE [[gnu::pure]]
#define NODISCARD [[nodiscard]]