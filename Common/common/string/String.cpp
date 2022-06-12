#include "String.h"
#include <algorithm>
#include <codecvt>
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

    bool Empty(std::string_view value)
    {
        for (const char& c : value)
            if (c != '\t' && c != ' ')
                return false;

        return true;
    }

    std::string_view TrimWhitespace(std::string_view value)
    {
        //Trim front
        while (value.size() > 0 && (value.front() == ' ' || value.front() == '\t'))
            value.remove_prefix(1);
        //Trim back
        while (value.size() > 0 && (value.back() == ' ' || value.back() == '\t'))
            value.remove_suffix(1);

        return value;
    }

#pragma warning(disable:4996) //Disable warning about <codecvt> being deprecated. Fine for now since MSVC still lets us use it in c++20 and there's no standard replacement.
    //Source: https://stackoverflow.com/a/49741944
    std::wstring ToWideString(const std::string& in)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(in);
    }

    std::string ToNarrowString(const std::wstring& in)
    {
        //Convert strings from std::wstring to std::string and cache them in the locale class
        using convert_type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_type, wchar_t> converter;
        return converter.to_bytes(in);
    }
#pragma warning(default: 4996)
}