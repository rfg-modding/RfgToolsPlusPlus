#include "common/Typedefs.h"
#include <string>
#include <vector>

namespace String
{
    bool EndsWith(s_view value, s_view ending);
    bool StartsWith(s_view value, s_view start);
    bool Contains(s_view value, s_view target);
    string ToLower(const string& value);
    std::vector<std::string_view> SplitString(std::string_view view, std::string_view delimiter);
    //Replace all instances of replace with replaceWith in value string. Returns result as new string
    string Replace(string value, const string& replace, const string& replaceWith);
    //Returns true if str0 and str1 are equal. Ignores case
    bool EqualIgnoreCase(const string& str0, const string& str1);
}