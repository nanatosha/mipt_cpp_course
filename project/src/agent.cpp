#include "agent.h"
#include <print>
#include <string>
#include "../kit/include/l3.1/rule.h"
#include "agent_rules.h"

namespace nano_edr {

Agent::Agent(bool quiet)
    : engine_(BuildAgentRules()), quiet_(quiet) {}


void Agent::HandleEvent(const Event& event) {
    ++events_;
    ++event_types_count_[event.type()];

    std::vector<const IRule*> fired;
    engine_.ProcessEvent(event, &fired);

    if (!fired.empty()) {
        if (!quiet_){
            if (window_.size() > 1){
                const WindowEntry& e = window_[window_.size() - 2];
                std::print("[CTX] -2: {} {} pid={}\n", e.ts.ms, e.type, e.pid);
            }
            if (window_.size() > 0){
                const WindowEntry& e = window_[window_.size() - 1];
                std::print("[CTX] -1: {} {} pid={}\n", e.ts.ms, e.type, e.pid);
            }
        }
        for (const auto* rule : fired) {
        std::print("[DETECT] {}  {}  ts={} pid={}\n",
                   SeverityName(rule->severity()), rule->id(),
                   event.raw_ts(), event.pid());
    }
    }

    window_.PushBack(WindowEntry{event.ts(), event.type(), event.pid()});
}

void Agent::PrintSummary() const {
    if (quiet_) {
        return;
    }
    std::println("событий всего : {}", events_);
    std::println("пропущено строк : {}", skipped_);
    std::println("битых строк : {}", broken_);
    for (const auto& [type, count] : event_types_count_) {
        std::println("событий типа {} всего : {}", type, count);
    }
    std::println("правила, детектов:");
    for (std::size_t i = 0; i < engine_.size(); ++i) {
        std::println("  {}  {}", engine_.rule(i).id(), engine_.rule(i).hits());
    }
    std::println("[STAT] в окне {} из {} мест", window_.size(), kWindowCapacity);
}

}  // namespace nano_edr