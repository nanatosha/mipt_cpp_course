#include "../kit/include/l1.3/rules.h"

#include <print>

namespace nano_edr {

const char* SeverityName(Severity severity) {
    switch (severity) {
        case Severity::kLow: {
            return "low";
        }
        case Severity::kMedium: {
            return "medium";
        }
        case Severity::kHigh: {
            return "high";
        }
        case Severity::kCritical: {
            return "critical";
        }
    }
    return "?";
}

size_t CheckRules(const Event& event, const Rule* rules, size_t rule_count) {
    size_t detections = 0;
    for (size_t i = 0; i < rule_count; ++i) {
        if (rules[i].check(event)) {
            std::print("[DETECT] {}  {}  ts={} pid={}\n",
                       SeverityName(rules[i].severity), rules[i].id,
                       event.ts, event.pid);
            ++detections;
        }
    }
    return detections;
}

}  