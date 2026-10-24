#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H
#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include "../kit/include/l2.3/event.h"
#include "../kit/include/l2.3/event_list.h"
#include "../kit/include/l2.3/event_source.h"
#include "../kit/include/l2.3/rule_engine.h"

namespace nano_edr {
class Agent : public IEventHandler {
public:
    Agent(std::size_t window_size, bool quiet);
    
    void HandleEvent(const Event& event) override;
    void OnSkipped() override { ++skipped_; }
    void OnBroken() override { ++broken_; }
    void OnTick() override {}
    
    void PrintSummary() const;

private:
    const EventNode* prev1_ = nullptr;
    const EventNode* prev2_ = nullptr;
    EventList window_;
    RuleEngine engine_;
    bool quiet_;
    
    std::size_t events_ = 0;
    std::size_t skipped_ = 0;
    std::size_t broken_ = 0;
    std::map<std::string, std::size_t> event_types_count_;
};
}  // namespace nano_edr
#endif