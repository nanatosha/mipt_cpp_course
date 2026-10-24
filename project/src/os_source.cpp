#include "os_source.h"
#include <stdexcept>
#include "../kit/include/l2.3/event.h"

namespace nano_edr {

OsSource::OsSource(const std::string& config_path)
    : handle_(config_path) {}

void OsSource::Trampoline(const os_event* ev, void* ctx) noexcept {
    IEventHandler* handler = static_cast<IEventHandler*>(ctx);
    try {
        EventParts parts;
        parts.ts = std::to_string(ev->ts);
        parts.type = ev->type ? ev->type : "";
        parts.pid = ev->pid == 0 ? std::string{} : std::to_string(ev->pid);
        parts.fields.reserve(ev->field_count);
        for (std::size_t i = 0; i < ev->field_count; ++i) {
            parts.fields.push_back(Field{ev->fields[i].key, ev->fields[i].value});
        }
        Event event(parts);
        handler->HandleEvent(event);
    } catch (...) {
        handler->OnBroken();
    }
}

void OsSource::Run(IEventHandler* handler) {
    handle_.Subscribe(&OsSource::Trampoline, handler);
    handle_.Start();
    
    while (true) {
        os_status status = handle_.Wait(1000);
        if (status == OS_OK) {
            break;
        }
        if (status == OS_TIMEOUT) {
            handler->OnTick();
            continue;
        }
        throw std::runtime_error("ошибка при ожидании событий: " +
                                 std::string(os_status_str(status)));
    }
    handle_.Stop();
}

}  // namespace nano_edr