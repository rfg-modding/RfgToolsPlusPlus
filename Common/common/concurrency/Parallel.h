#pragma once
#include <future>
#include <type_traits>

namespace parallel
{
    ////Simple wrapper around std::async to ensure it's run asynchronously. Based on code from Effective Modern C++, page 249
    //template<typename F, typename... Ts>
    //inline std::future<typename std::result_of<F(Ts...)>::type> AsyncFunction(F&& f, Ts&&... params) // return future
    //{
    //    return std::async(std::launch::async, // call to f(params...)
    //                      std::forward<F>(f),
    //                      std::forward<Ts>(params)...);
    //}
}