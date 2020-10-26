#include "String.h"
#include <algorithm>

namespace String
{
    bool EndsWith(s_view value, s_view ending)
    {
        if (value.length() >= ending.length())
            return value.compare(value.length() - ending.length(), ending.length(), ending) == 0;
        else
            return false;
    }

    bool StartsWith(s_view value, s_view start)
    {
        return value.compare(0, start.length(), start) == 0;
    }

    bool Contains(s_view value, s_view target)
    {
        return value.find(target) != std::string_view::npos;
    }

    string ToLower(const string& value)
    {
        std::string Copy = value; //Copy the string since we want to leave the original string intact
        std::transform(Copy.begin(), Copy.end(), Copy.begin(), [](unsigned char c) { return std::tolower(c); });
        return Copy;
    }
}