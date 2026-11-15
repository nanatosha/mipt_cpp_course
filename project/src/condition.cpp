#include "../kit/include/l2.3/conditions.h"
#include "../kit/include/l2.3/fields.h"
#include "text.h"

namespace nano_edr {

bool FieldEquals::Matches(const Event& event) const {
    const std::string* field_value = FindField(event, key_);
    if (!field_value) return false;
    return EqualsIgnoreCase(*field_value, value_);
}

bool FieldContains::Matches(const Event& event) const {
    const std::string* field_value = FindField(event, key_);
    if (!field_value) return false;
    return ContainsIgnoreCase(*field_value, needle_);
}

bool FieldEndsWith::Matches(const Event& event) const {
    const std::string* field_value = FindField(event, key_);
    if (!field_value) return false;
    return EndsWithIgnoreCase(*field_value, suffix_);
}

bool FieldInList::Matches(const Event& event) const {
    const std::string* field_value = FindField(event, key_);
    if (!field_value) return false;
    for (const std::string& value : values_) {
        if (EqualsIgnoreCase(*field_value, value)) return true;
    }
    return false;
}

bool EventTypeIs::Matches(const Event& event) const {
    for (const auto& type : types_) {
        if (event.event_type() == type) return true;
    }
    return false;
}

void AllOf::Add(std::unique_ptr<ICondition> condition) {
    parts_.push_back(std::move(condition));
}

bool AllOf::Matches(const Event& event) const {
    for (const auto& condition : parts_) {
        if (!condition->Matches(event)) return false;
    }
    return true;
}

void AnyOf::Add(std::unique_ptr<ICondition> condition) {
    parts_.push_back(std::move(condition));
}

bool AnyOf::Matches(const Event& event) const {
    for (const auto& condition : parts_) {
        if (condition->Matches(event)) return true;
    }
    return false;
}

}  // namespace nano_edr