#include "agent.h"
#include <print>
#include <string>
#include "../kit/include/l3.2/rule.h"
#include "agent_rules.h"
#include "../kit/include/l3.2/alert_sink.h"
#include "dedup_sink.h"

namespace nano_edr {

Agent::Agent(bool quiet, uint64_t dedup_period_ms)
    : engine_(BuildAgentRules(&model_)),
      quiet_(quiet),
      dedup_period_ms_(dedup_period_ms) {
    engine_.SetModel(&model_);
}
void Agent::HandleEvent(const Event& event) {
    model_.Observe(event);
    context_printed_ = false;
    ++events_;
    ++event_types_count_[event.type()];

    std::vector<const IRule*> fired;
    engine_.ProcessEvent(event, &fired);

    

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
    std::println("[STAT] в модели процессов {}, файлов {}", model_.process_count(), model_.file_count());
    if (suppressed_ > 0) {
        std::println("[STAT] подавлено дубликатов {} (срок {} мс)", suppressed_, dedup_period_ms_);
    }

}

void Agent::Subscribe(AlertSink sink) {
    FanOutSink all;
    all.Add([this](const Detection&) { PrintContextOnce(); });
    all.Add(std::move(sink));
    // было: engine_.Subscribe(std::move(all));
    engine_.Subscribe(DedupSink(std::move(all), dedup_period_ms_, &suppressed_));
}

void Agent::PrintContextOnce() {
    if (!context_printed_){
        if(quiet_) {
            return;
        }
        context_printed_ = true;
        if (window_.size() > 1) {
            const WindowEntry& e = window_[window_.size() - 2];
            std::print("[CTX] -2: {} {} pid={}\n", e.ts.ms, e.type, e.pid);
        }
        if (window_.size() > 0) {
            const WindowEntry& e = window_[window_.size() - 1];
            std::print("[CTX] -1: {} {} pid={}\n", e.ts.ms, e.type, e.pid);
        }
    }
}

}  // namespace nano_edr