#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H
#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include "../kit/include/l3.2/event.h"
#include "../kit/include/l3.2/event_list.h"
#include "../kit/include/l3.2/event_source.h"
#include "../kit/include/l3.2/rule_engine.h"
#include "ring_buffer.h"
#include "../kit/include/l3.2/alert_sink.h"
   #include "../kit/include/l3.2/entity_model.h"


namespace nano_edr {

struct WindowEntry {
    Timestamp ts;
    std::string type;
    std::string pid;
};

class Agent : public IEventHandler {
public:
    explicit Agent(bool quiet, uint64_t dedup_period_ms);
    
    void HandleEvent(const Event& event) override;
    void OnSkipped() override { ++skipped_; }
    void OnBroken() override { ++broken_; }
    void OnTick() override {}
    
    void PrintSummary() const;
    static constexpr std::size_t kWindowCapacity = 256;
    void Subscribe(AlertSink sink);

private:
    bool context_printed_ = false;
    RingBuffer<WindowEntry, kWindowCapacity> window_;
    EntityModel model_;
    RuleEngine engine_;
    bool quiet_;
    
    std::size_t events_ = 0;
    std::size_t skipped_ = 0;
    std::size_t broken_ = 0;
    std::map<std::string, std::size_t> event_types_count_;

    void PrintContextOnce();
    uint64_t dedup_period_ms_{};
    std::size_t suppressed_{};
};
}  // namespace nano_edr
#endif