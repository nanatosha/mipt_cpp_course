#include "agent.h"
#include <print>
#include <string>
#include "../kit/include/l2.3/rule.h"
#include "agent_rules.h"

namespace nano_edr {

Agent::Agent(std::size_t window_size, bool quiet)
    : window_(window_size), engine_(BuildAgentRules()), quiet_(quiet) {}

void Agent::HandleEvent(const Event& event) {
    ++events_;
    ++event_types_count_[event.type()];
    
    prev2_ = prev1_;
    prev1_ = window_.back();
    window_.PushBack(event);
    
    std::vector<const IRule*> fired;
    engine_.ProcessEvent(event, &fired);
    
    for (const auto* rule : fired) {
        std::print("[DETECT] {}  {}  ts={} pid={}\n",
                   SeverityName(rule->severity()), rule->id(),
                   event.raw_ts(), event.pid());
    }
    
    if (!fired.empty()) {
        if (prev2_) {
            std::print(stderr, "[CTX] {}\n", prev2_->event);
        }
        if (prev1_) {
            std::print(stderr, "[CTX] {}\n", prev1_->event);
        }
    }
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
}

}  // namespace nano_edr