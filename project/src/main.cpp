#include <charconv>
#include <fstream>
#include <map>
#include <print>
#include <string>
#include <string_view>

#include "../kit/include/l1.2/event_list.h"
#include "../kit/include/l1.2/parse.h"
#include "../kit/include/l1.3/rules.h"
#include "agent_rules.h"

namespace nano_edr {
namespace {

struct Options {
    std::string log_path;
    bool is_quiet = false;
    std::size_t window_size = 64;
};


bool ParseArgs(int argc, char** argv, Options* out) {
    if (argc < 2) {
        return false;
    }
    out->log_path = argv[1];
    for (int i = 2; i < argc; ++i) {
        std::string_view a = argv[i];
        if (a == "--quiet") {
            out->is_quiet = true;
        } 
        else if (a == "--window-size" && i + 1 < argc) {
            std::string_view s = argv[i + 1];
            long long n = 0;
            auto parsed = std::from_chars(s.data(), s.data() + s.size(), n);
            if (parsed.ec == std::errc()) {
                out->window_size = static_cast<std::size_t>(n);
                ++i;
            }
        }
    }
    return true;
}

void PrintContext(const EventList& window) {
    EventNode* last1 = nullptr;
    EventNode* last2 = nullptr;
    for (EventNode* n = window.head; n; n = n->next) {
        last1 = last2;
        last2 = n;
    }
    if (last1) {
        std::print("[CTX] {}: ts={} type={} pid={}\n", -2, last1->event.ts,
                   last1->event.type, last1->event.pid);
    }
    if (last2) {
        std::print("[CTX] {}: ts={} type={} pid={}\n", -1, last2->event.ts,
                   last2->event.type, last2->event.pid);
    }
}

void PrintSummary(long long lines, long long comments, long long events, const std::map<std::string, long long>& event_types_count) {
    std::println("строк {}, из них комментариев {}", lines, comments);
    std::println("событий всего : {}", events);
    for (const auto& [type, count] : event_types_count) {
        std::println("событий типа {} всего : {}", type, count);
    }
}


void RunLog(const Options& options) {
    std::ifstream log(options.log_path);
    if (!log) {
        throw std::runtime_error("не удалось открыть журнал: " + options.log_path);
    }

    EventList window{.capacity = options.window_size};

    long long lines = 0;
    long long comments = 0;
    long long events = 0;
    std::map<std::string, long long> event_types_count;
    std::string line;

    while (std::getline(log, line)) {
        ++lines;
        if (IsBlankOrComment(&line)) {
            if (!line.empty()) {
                ++comments;
            }
            continue;
        }

        Event ev;
        if (!ParseEventLine(&line, &ev)) {
            continue;
        }

        ++events;
        ++event_types_count[ev.type];

        size_t detections = CheckRules(ev, AgentRules(), AgentRuleCount());

        if (detections > 0 && !options.is_quiet) {
            PrintContext(window);
        }

        ListPushBack(&window, &ev);
    }

    if (!options.is_quiet) {
        PrintSummary(lines, comments, events, event_types_count);
    }
}

}  
}  

int main(int argc, char** argv) {
    nano_edr::Options options;
    if (!nano_edr::ParseArgs(argc, argv, &options)) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return 2;
    }

    try {
        nano_edr::RunLog(options);
    } catch (const std::exception& error) {
        std::print(stderr, "прогон оборван: {}\n", error.what());
        return 1;
    }

    return 0;
}