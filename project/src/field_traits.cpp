#include "../kit/include/l3.1/field_traits.h"
#include <charconv>
#include "text.h"

namespace nano_edr {

bool FieldTraits<std::string>::Parse(const std::string& s, std::string* out) {
    *out = s;
    return true;
}
bool FieldTraits<uint64_t>::Parse(const std::string& s, uint64_t* out) {
    uint64_t n = 0;
    auto parsed = std::from_chars(s.data(), s.data() + s.size(), n);
    if (parsed.ec != std::errc() || parsed.ptr != s.data() + s.size()){
        return false;
    }
    *out = n;
    return true;
}
bool FieldTraits<bool>::Parse(const std::string& s, bool* out) {
    if (EqualsIgnoreCase(s, "1") || EqualsIgnoreCase(s, "true") || EqualsIgnoreCase(s, "yes")){
        *out = true;
        return true;
    }
    else if (EqualsIgnoreCase(s, "0") || EqualsIgnoreCase(s, "false") || EqualsIgnoreCase(s, "no")){
        *out = false;
        return true;
    }
    return false;
}

}  // namespace nano_edr
