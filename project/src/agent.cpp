#include "agent.h"

#include <print>
#include <string>

#include "../kit/include/l2.1/rules.h"
#include "agent_rules.h"

namespace nano_edr {

Agent::Agent(std::size_t window_size, bool quiet)
    : window_(window_size), quiet_(quiet) {}

void Agent::HandleEvent(const Event& event) {
    ++events_;
    ++event_types_count_[event.type()];
    
  
    size_t detections = CheckRules(event, AgentRules(), AgentRuleCount());
    window_.PushBack(event);

    if (detections > 0) {
        const EventNode* prev = nullptr;
        const EventNode* current = window_.head();
        while (current && current->next) {
            prev = current;
            current = current->next;
        }
        if (prev) {
            std::print(stderr, "[CTX] {}\n", prev->event);
        }
        if (current) {
            std::print(stderr, "[CTX] {}\n", current->event);
        }
    }
}

void Agent::PrintSummary() const {
  if (quiet_) {
    return;
  }
  std::println("событий всего : {}", events_);
  std::println("событий отброшено : {}", dropped_);
  std::println("битых строк : {}", bad_lines_);
  for (const auto& [type, count] : event_types_count_) {
    std::println("событий типа {} всего : {}", type, count);
  }
}

void Agent::Trampoline(const os_event* ev, void* ctx) noexcept {
  try {
    Agent* agent = static_cast<Agent*>(ctx);

    EventParts parts;
    parts.ts = std::to_string(ev->ts);
    parts.type = ev->type ? ev->type : "";
    parts.pid = ev->pid == 0 ? std::string{} : std::to_string(ev->pid);
    parts.fields.reserve(ev->field_count);
    for (std::size_t i = 0; i < ev->field_count; ++i) {
      parts.fields.push_back(Field{ev->fields[i].key, ev->fields[i].value});
    }

    Event event(parts);
    agent->HandleEvent(event);
  } catch (...) {
  }
}

}  // namespace nano_edr