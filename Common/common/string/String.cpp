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

    std::vector<std::string_view> SplitString(std::string_view view, std::string_view delimiter)
    {
        std::vector<std::string_view> output;
        //output.reserve(view.length() / 4);
        auto first = view.begin();

        while (first != view.end())
        {
            const auto second = std::find_first_of(first, std::cend(view), std::cbegin(delimiter), std::cend(delimiter));
            if (first != second)
            {
                output.emplace_back(view.substr(std::distance(view.begin(), first), std::distance(first, second)));
            }

            if (second == view.end())
                break;

            first = std::next(second);
        }

        return output;
    }
}