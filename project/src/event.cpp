#include "../kit/include/l2.3/event.h"
#include <charconv>
#include <sstream>
#include <stdexcept>

namespace nano_edr {


EventType EventTypeFromString(const std::string& type) {
    if (type == "process_start") return EventType::kProcessStart;
    if (type == "process_end") return EventType::kProcessEnd;
    if (type == "file_create") return EventType::kFileCreate;
    if (type == "file_write") return EventType::kFileWrite;
    if (type == "file_delete") return EventType::kFileDelete;
    if (type == "file_move") return EventType::kFileMove;
    if (type == "net_connect") return EventType::kNetConnect;
    return EventType::kOther;  // неизвестный тип
}


Event::Event(const EventParts& parts) {
  // Проверка ts
  if (parts.ts.empty()) {
    throw std::invalid_argument("неверное время события: (пусто)");
  }
  uint64_t ts_value = 0;
  auto result = std::from_chars(parts.ts.data(),
                                parts.ts.data() + parts.ts.size(),
                                ts_value);
  if (result.ec != std::errc() || result.ptr != parts.ts.data() + parts.ts.size()) {
    throw std::invalid_argument("неверное время события: " + parts.ts);
  }
  // Проверка type
  if (parts.type.empty()) {
    throw std::invalid_argument("неверный тип события: (пусто)");
  }
  ts_ = Timestamp{ts_value};
  raw_ts_ = parts.ts;
  type_ = parts.type;
  event_type_ = EventTypeFromString(parts.type);
  pid_ = parts.pid;
  fields_ = parts.fields;
}

std::string ToString(const Event& event) {
  std::ostringstream oss;
  oss << "ts=" << event.raw_ts()
      << " type=" << event.type()
      << " pid=" << event.pid();
  for (const auto& field : event.fields()) {
    oss << " " << field.key << "=" << field.value;
  }
  return oss.str();
}

std::ostream& operator<<(std::ostream& out, const Event& event) {
  return out << ToString(event);
}


}  // namespace nano_edr