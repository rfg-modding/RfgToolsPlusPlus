#pragma once
#include <cstdint>
#include <string>
#include <memory>

//Signed ints
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

//Unsigned ints
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

//Floating point values
using f32 = float;
using f64 = double;

//Misc types
using string = std::string;
using s_view = std::string_view;

//Shorter / more useful name for std::shared_ptr
template<typename T>
using Handle = std::shared_ptr<T>;
template<typename T, typename... Args>
constexpr Handle<T> CreateHandle(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

static_assert(sizeof(i8) == 1);
static_assert(sizeof(i16) == 2);
static_assert(sizeof(i32) == 4);
static_assert(sizeof(i64) == 8);

static_assert(sizeof(u8) == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u64) == 8);

static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

//Defer statement like zig. Useful since you can put cleanup code next to init code.
//E.g. buffer will be de-allocated once it goes out of scope:
//    u8* buffer = new u8[4096];
//    defer(delete[] buffer);
//Implementation based on these articles:
//    - https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/
//    - http://the-witness.net/news/2012/11/scopeexit-in-c11/

//Calls provided lambda on destruction
template<typename F>
struct DeferInstance
{
    F Func;
    DeferInstance(F func) : Func(func) {}
    ~DeferInstance() { Func(); }
};

template<typename F>
DeferInstance<F> CreateDeferInstance(F func)
{
    return DeferInstance<F>(func);
}

//Macros create DeferInstance instances with unique names which call the enclosed code at the end of scope
#define DEFER_PRIVATE_1(x, y) x##y
#define DEFER_PRIVATE_2(x, y) DEFER_PRIVATE_1(x, y)
#define DEFER_PRIVATE_3(x)    DEFER_PRIVATE_2(x, __COUNTER__)
#define defer(code)   auto DEFER_PRIVATE_3(_defer_) = CreateDeferInstance([&](){code;})