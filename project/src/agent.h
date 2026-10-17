#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H

#include <cstddef>
#include <map>
#include <string>

#include "../kit/include/l2.1/event.h"
#include "../kit/include/l2.1/event_list.h"
#include "os_handle.h"

namespace nano_edr {

class Agent {
 public:
  Agent(std::size_t window_size, bool quiet);

  void HandleEvent(const Event& event);
  void PrintSummary() const;

  static void Trampoline(const os_event* ev, void* ctx) noexcept;

  void ReportDropped() { ++dropped_; } 
  void ReportBadLine() { ++bad_lines_; }

 private:
  const EventNode* prev1_ = nullptr;
  const EventNode* prev2_ = nullptr;
  EventList window_;
  bool quiet_;
  std::size_t events_ = 0;
  std::size_t dropped_ = 0;
  std::size_t bad_lines_ = 0;
  std::map<std::string, std::size_t> event_types_count_;
};

}  // namespace nano_edr

#endif  // NANO_EDR_AGENT_H