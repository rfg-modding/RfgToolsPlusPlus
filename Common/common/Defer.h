#pragma once

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