#pragma once
#include <string_view>
#include <string>

//This file is temporary. Previously these two typedefs were in Typedefs.h.
//The problem is that this includes <string> and <string_view> in every file that includes Typedefs.h.
//This was moved out to reduce compile time. Eventually the codebase should just be changed to use the actual type names

//Misc types
using string = std::string;
using s_view = std::string_view;