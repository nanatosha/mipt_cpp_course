

#include "../kit/include/l2.2/fields.h"

#include <charconv>
#include <cctype>
#include <stdexcept>

namespace nano_edr {

namespace {

std::string LowReg(const std::string& value) {
    std::string result = value;
    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

}  

const std::string* FindField(const Event& event, const std::string& key) {
    if (key == "ts") {
        return &event.raw_ts();
    }
    if (key == "type") {
        return &event.type();
    }
    if (key == "pid") {
        return &event.pid();
    }
    for (const Field& field : event.fields()) {
        if (field.key == key) {
            return &field.value;
        }
    }
    return nullptr;
}

const std::string& GetRequiredField(const Event& event, const std::string& key) { //throw std::invalid_argument, т.к. поле обяхательно
    const std::string* value = FindField(event, key);
    if (!value) {
        throw std::invalid_argument("обязательное поле отсутствует: " + key);
    }
    return *value;
}

bool GetIntField(const Event& event, const std::string& key, uint64_t* out) { //bool / fallback, битые числа бывают
    const std::string* value = FindField(event, key);
    if (!value) {
        return false;
    }
    uint64_t parsed_value = 0;
    const char* begin = value->data();
    const char* end = value->data() + value->size();
    auto result = std::from_chars(begin, end, parsed_value);
    if (result.ec != std::errc() || result.ptr != end) {
        return false;
    }
    *out = parsed_value;
    return true;
}

uint64_t GetIntField(const Event& event, const std::string& key, uint64_t fallback) { // false если поля нет или не удалось распарсить строку
    uint64_t value = 0;
    if (GetIntField(event, key, &value)) {
        return value;
    }
    return fallback;
}

bool IsProcessStart(const Event& event) {
    return event.type() == "process_start";
}

bool IsFileWrite(const Event& event) {
    return event.type() == "file_write";
}

bool IsNetConnect(const Event& event) {
    return event.type() == "net_connect";
}

std::string NormalizePath(const std::string& path) { //всегда ответ да/нет
    std::string result = path;

    for (char& c : result) {
        if (c == '/') {
            c = '\\';
        }
    }

    result = LowReg(result);

    const std::string kExpanded = "\\appdata\\local\\temp";
    for (const std::string& token : {std::string("%temp%"), std::string("%tmp%")}) {
        std::size_t pos = 0;
        while ((pos = result.find(token, pos)) != std::string::npos) {
            result.replace(pos, token.size(), kExpanded);
            pos += kExpanded.size();
        }
    }

    std::string collapsed;
    collapsed.reserve(result.size());
    for (char c : result) {
        if (c == '\\' && !collapsed.empty() && collapsed.back() == '\\') {
            continue;
        }
        collapsed.push_back(c);
    }

    return collapsed;
}

bool PathEndsWith(const Event& event, const std::string& suffix) {
    const std::string* path = FindField(event, "path");
    if (!path) {
        return false;
    }
    std::string normalized_path = NormalizePath(*path);
    std::string normalized_suffix = NormalizePath(suffix);
    if (normalized_path.size() < normalized_suffix.size()) {
        return false;
    }
    return normalized_path.compare(normalized_path.size() - normalized_suffix.size(),
                                    normalized_suffix.size(), normalized_suffix) == 0;
}

bool CommandLineContains(const Event& event, const std::string& needle) {
    const std::string* cmdline = FindField(event, "cmdline");
    if (!cmdline) {
        return false;
    }
    std::string normalized_cmdline = NormalizePath(*cmdline);
    std::string normalized_needle = NormalizePath(needle);
    return normalized_cmdline.find(normalized_needle) != std::string::npos;
}

}