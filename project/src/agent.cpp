#include "agent.h"

#include <print>
#include <string>

#include "../kit/include/l2.1/rules.h"
#include "agent_rules.h"

namespace nano_edr {

namespace{
  Event Converter(const os_event* ev){
    EventParts parts;
    parts.ts = std::to_string(ev->ts);
    parts.type = ev->type ? ev->type : "";
    parts.pid = ev->pid == 0 ? std::string{} : std::to_string(ev->pid);
    parts.fields.reserve(ev->field_count);
    for (std::size_t i = 0; i < ev->field_count; ++i) {
      parts.fields.push_back(Field{ev->fields[i].key, ev->fields[i].value});
    }

    Event event(parts);
    return event;
  }
}

Agent::Agent(std::size_t window_size, bool quiet)
    : window_(window_size), quiet_(quiet) {}

void Agent::HandleEvent(const Event& event) {
    ++events_;
    ++event_types_count_[event.type()];
    
  
    size_t detections = CheckRules(event, AgentRules(), AgentRuleCount());
    
    prev2_ = prev1_;
    prev1_ = window_.back();
    
    window_.PushBack(event);

    if (detections > 0) {
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
  std::println("событий отброшено : {}", dropped_);
  std::println("битых строк : {}", bad_lines_);
  for (const auto& [type, count] : event_types_count_) {
    std::println("событий типа {} всего : {}", type, count);
  }
}

void Agent::Trampoline(const os_event* ev, void* ctx) noexcept {
  Agent* agent = static_cast<Agent*>(ctx);
  try {

    agent->HandleEvent(Converter(ev));
  } 
  catch (const std::exception& error) {
    agent->ReportDropped();
  }
  catch (...) {
    agent->ReportDropped();
  }
}

}  // namespace nano_edr