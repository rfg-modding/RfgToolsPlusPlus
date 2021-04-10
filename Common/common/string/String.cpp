#include "String.h"
#include <algorithm>
#include <regex>

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

    string ToLower(s_view value)
    {
        std::string Copy(value); //Copy the string since we want to leave the original string intact
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

    string Replace(string value, const string& replace, const string& replaceWith)
    {
        return std::regex_replace(value, std::regex(replace), replaceWith);
    }

    bool EqualIgnoreCase(const string& str0, const string& str1)
    {
        string str0Lower = String::ToLower(str0);
        string str1Lower = String::ToLower(str1);
        return str0Lower == str1Lower;
    }
    bool EqualIgnoreCase(s_view str0, s_view str1)
    {
        string str0Lower = String::ToLower(str0);
        string str1Lower = String::ToLower(str1);
        return str0Lower == str1Lower;
    }
    size_t FindNthCharacterFromBack(s_view str, char character, u32 n)
    {
        size_t targetPos = 0;
        u32 foundCount = 0;
        for (size_t i = str.size() - 1; i > 0; i--)
        {
            if (str[i] == '/')
            {
                foundCount++;
                if (foundCount == n)
                    targetPos = i;
            }
        }
        return targetPos;
    }
}