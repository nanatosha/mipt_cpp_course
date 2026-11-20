#include "../kit/include/l3.2/rule_engine.h"


namespace nano_edr {

void RuleEngine::AddRule(std::unique_ptr<IRule> rule) {
    rules_.push_back(std::move(rule));
}

std::size_t RuleEngine::ProcessEvent(const Event& event,
                                     std::vector<const IRule*>* fired) {
    std::size_t count = 0;
    for (const auto& rule : rules_) {
        if (rule->Check(event)) {
            Detection detection = RuleEngine::MakeDetection(*rule, event);
            for (const auto& sink : sinks_) {
                sink(detection);
            }
            ++count;
            if (fired) {
                fired->push_back(rule.get());
            }
        }
    }
    return count;
}

void RuleEngine::Subscribe(AlertSink sink) {
    sinks_.push_back(std::move(sink));
}

Detection RuleEngine::MakeDetection(const IRule& rule, const Event& event) const{
    Detection detection;
    detection.severity = rule.severity();
    detection.rule = rule.id();
    detection.ts = event.ts();
    detection.pid = event.pid();
    detection.actions = rule.actions();

    if (model_ != nullptr) {
        detection.chain = model_->DescribeChain(event.pid());
    }

    return detection;

}
}  // namespace nano_edr