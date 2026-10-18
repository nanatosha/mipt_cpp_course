// text.cpp
#include "text.h"

#include <algorithm>
#include <cctype>

namespace nano_edr {

namespace {
std::string ToLower(const std::string& s) {
    std::string result = s;
    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}
}  // namespace

bool EqualsIgnoreCase(const std::string& a, const std::string& b) {
    return ToLower(a) == ToLower(b);
}

bool ContainsIgnoreCase(const std::string& haystack, const std::string& needle) {
    return ToLower(haystack).find(ToLower(needle)) != std::string::npos;
}

bool EndsWithIgnoreCase(const std::string& s, const std::string& suffix) {
    return ToLower(s).ends_with(ToLower(suffix));
}

}  // nano_edr