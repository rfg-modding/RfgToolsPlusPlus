#pragma once
#include <memory>

//Shorter / more useful name for std::shared_ptr
template<typename T>
using Handle = std::shared_ptr<T>;
template<typename T, typename... Args>
constexpr Handle<T> CreateHandle(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}
