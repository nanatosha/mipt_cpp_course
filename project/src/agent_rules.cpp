#include "agent_rules.h"

#include "../kit/include/l1.3/fields.h"

namespace nano_edr {

namespace {

bool IsFileEventType(const Event& event) {
    return event.type == "file_create" || event.type == "file_write" || event.type == "file_move";
}

const std::string* FileDestinationPath(const Event& event) {
    if (event.type == "file_move") {
        return FindField(event, "to");
    }
    return FindField(event, "path");
}

bool ScriptHostFromTemp(const Event& event) {
    if (!IsProcessStart(event)) {
        return false;
    }
    std::string image = NormalizePath(GetRequiredField(event, "image"));
    bool is_script_host = image.ends_with("\\wscript.exe") ||
                          image.ends_with("\\cscript.exe");
    if (!is_script_host) {
        return false;
    }
    const std::string* cmdline = FindField(event, "cmdline");
    if (!cmdline) {
        return false;
    }
    std::string normalized = NormalizePath(*cmdline);
    return normalized.find("\\appdata\\local\\temp\\") != std::string::npos || normalized.find("\\windows\\temp\\") != std::string::npos;
}

bool LolbinDownload(const Event& event) {
    if (!IsProcessStart(event)) {
        return false;
    }
    std::string image = NormalizePath(GetRequiredField(event, "image"));
    bool is_lolbin = image.ends_with("\\certutil.exe") ||
                     image.ends_with("\\bitsadmin.exe");
    if (!is_lolbin) {
        return false;
    }
    const std::string* cmdline = FindField(event, "cmdline");
    if (!cmdline) {
        return false;
    }
    std::string normalized = NormalizePath(*cmdline);
    return normalized.find("urlcache") != std::string::npos || normalized.find("transfer") != std::string::npos || normalized.find("http:") != std::string::npos || normalized.find("https:") != std::string::npos;
}

bool HiddenPowershell(const Event& event) {
    if (!IsProcessStart(event)) {
        return false;
    }
    std::string image = NormalizePath(GetRequiredField(event, "image"));
    bool is_powershell = image.ends_with("\\powershell.exe") ||
                         image.ends_with("\\pwsh.exe");
    if (!is_powershell) {
        return false;
    }
    const std::string* cmdline = FindField(event, "cmdline");
    if (!cmdline) {
        return false;
    }
    std::string normalized = NormalizePath(*cmdline);
    return normalized.find("-w hidden") != std::string::npos || normalized.find("-windowstyle hidden") != std::string::npos || normalized.find("-enc") != std::string::npos;  
}

bool AutostartWrite(const Event& event) {
    if (!IsFileEventType(event)) {
        return false;
    }
    const std::string* dest = FileDestinationPath(event);
    if (!dest) {
        return false;
    }
    std::string normalized = NormalizePath(*dest);
    return normalized.find("\\start menu\\programs\\startup\\") != std::string::npos;
}

bool RansomExtension(const Event& event) {
    if (!IsFileEventType(event)) {
        return false;
    }
    const std::string* dest = FileDestinationPath(event);
    if (!dest) {
        return false;
    }
    std::string normalized = NormalizePath(*dest);
    return normalized.ends_with(".locked");
}

constexpr Rule kRules[] = {
    {"script_host_from_temp", ScriptHostFromTemp, Severity::kHigh},
    {"lolbin_download", LolbinDownload, Severity::kHigh},
    {"hidden_powershell", HiddenPowershell, Severity::kMedium},
    {"autostart_write", AutostartWrite, Severity::kHigh},
    {"ransom_extension", RansomExtension, Severity::kCritical},
};

}  

const Rule* AgentRules() {
    return kRules;
}

size_t AgentRuleCount() {
    return sizeof(kRules) / sizeof(kRules[0]);
}

}  