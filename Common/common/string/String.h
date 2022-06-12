#include "common/Typedefs.h"
#include "common/String.h"
#include <string>
#include <vector>

namespace String
{
    bool EndsWith(s_view value, s_view ending);
    bool StartsWith(s_view value, s_view start);
    bool Contains(s_view value, s_view target);
    string ToLower(const string& value);
    string ToLower(s_view value);
    std::vector<std::string_view> SplitString(std::string_view view, std::string_view delimiter);
    //Replace all instances of replace with replaceWith in value string. Returns result as new string
    string Replace(string value, const string& replace, const string& replaceWith);
    //Returns true if str0 and str1 are equal. Ignores case
    bool EqualIgnoreCase(const string& str0, const string& str1);
    bool EqualIgnoreCase(s_view str0, s_view str1);
    //Get nth character from the back of str. E.g. FindNthCharacterFromBack("a/a/a/", '/', 2) would return 3
    size_t FindNthCharacterFromBack(s_view str, char character, u32 n);
    bool Empty(std::string_view value);
    std::string_view TrimWhitespace(std::string_view value);

    //Convert std::string to std::wstring
    std::wstring ToWideString(const std::string& in);
    //Convert std::wstring to std::string
    std::string ToNarrowString(const std::wstring& in);
}