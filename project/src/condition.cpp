#include "../kit/include/l2.2/conditions.h"
#include "../kit/include/l2.2/fields.h"
#include "text.h"

namespace nano_edr {

bool FieldEquals::operator()(const Event& event) const {
    const std::string* field_value = FindField(event, key_);
    if (!field_value) {
        return false;
    }
    return EqualsIgnoreCase(*field_value, value_);
}

bool FieldContains::operator()(const Event& event) const {
    const std::string* field_value = FindField(event, key_);
    if (!field_value) {
        return false;
    }
    return ContainsIgnoreCase(*field_value, needle_);
}

bool FieldEndsWith::operator()(const Event& event) const {
    const std::string* field_value = FindField(event, key_);
    if (!field_value) {
        return false;
    }
    return EndsWithIgnoreCase(*field_value, suffix_);
}

bool FieldInList::operator()(const Event& event) const {
    const std::string* field_value = FindField(event, key_);
    if (!field_value) {
        return false;
    }
    for (const std::string& value : values_){
        if (EqualsIgnoreCase(*field_value, value)){
            return true;
        }
    }
    return false;
}

Condition Condition::Equals(const std::string& key,
                            const std::vector<std::string>& values) {
  return Condition(Match::kEquals, key, values);
}

Condition Condition::Contains(const std::string& key,
                              const std::vector<std::string>& values) {
  return Condition(Match::kContains, key, values);
}

Condition Condition::EndsWith(const std::string& key,
                              const std::vector<std::string>& values) {
  return Condition(Match::kEndsWith, key, values);
}

bool Condition::operator()(const Event& event) const {
  const std::string* field_value = FindField(event, key_);
  if (!field_value) {
    return false;
  }
  for (const std::string& value : values_) {
    switch (match_) {
      case Match::kEquals:
        if (EqualsIgnoreCase(*field_value, value)) {
          return true;
        }
        break;
      case Match::kContains:
        if (ContainsIgnoreCase(*field_value, value)) {
          return true;
        }
        break;
      case Match::kEndsWith:
        if (EndsWithIgnoreCase(*field_value, value)) {
          return true;
        }
        break;
    }
  }
  return false;
}


}  // namespace nano_edr