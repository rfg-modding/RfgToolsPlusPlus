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
}