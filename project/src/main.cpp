#include <algorithm>
#include <array>
#include <charconv>
#include <cstdio>
#include <fstream>
#include <map>
#include <print>
#include <string>
#include <string_view>
#include <vector>

#include "../kit/include/l1.2/parse.h"
#include "../kit/include/l1.2/event_list.h"


const std::vector<std::string> signatures = {
    "wscript.exe",
    ".locked",
    "certutil.exe",
    "\\Startup\\",
}; 

int main(int argc, char** argv) {

    bool is_quiet  = false;
    long long window_size = 64;

    if (argc < 2) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return 2;
    }

    std::ifstream log(argv[1]);
    if (!log) {
        std::print(stderr, "не удалось открыть журнал: {}\n", argv[1]);
        return 2;
    }

    for (int i = 2; i < argc; ++i) {
        std::string_view a = argv[i];
        if (a == "--quiet") {
            is_quiet = true;
        } 
        else if (a == "--window-size" && i + 1 < argc) {  
            std::string_view s = argv[i + 1];
            long long n = 0;
            auto parsed = std::from_chars(s.data(), s.data() + s.size(), n);
            if (parsed.ec == std::errc() && n >= 0) {
                window_size = n;
                ++i;
            }
        }
    }

    nano_edr::EventList window{.capacity = (std::size_t)window_size};

    long long lines = 0, comments = 0, events = 0;
    std::map<std::string, long long> event_types_count;
    std::string line;

    while (std::getline(log, line)) {
        ++lines;
        if (nano_edr::IsBlankOrComment(&line)) {
            if (!line.empty()){
                comments++;
            }
            continue;
        }

        bool detected = false;
        for (auto si : signatures) {
            if (line.find(si) != std::string::npos) {
                std::print("[DETECT] строка {}, признак {}: {}\n", 
                    lines, si, line);
                detected = true;
            }
        }

        nano_edr::Event ev;
        if (!nano_edr::ParseEventLine(&line, &ev)) continue;

        ++events;
        ++event_types_count[ev.type];


        if (detected && !is_quiet) {
            std::vector<const nano_edr::Event*> recent;
            for (auto* n = window.head; n; n = n->next)
                recent.push_back(&n->event);

            int total = (int)recent.size();
            for (int k = std::max(0, total - 2); k < total; ++k) {
                const auto* e = recent[k];
                std::print("[CTX] {}: ts={} type={} pid={}\n",
                           k - total, e->ts, e->type, e->pid);
            }
        }

        ListPushBack(&window, &ev);
    }

    if (!is_quiet) {
        std::println("строк {}, из них комментариев {}", lines, comments);
        std::println("событий всего : {}", events);
        for (auto& [type, count] : event_types_count)
            std::println("событий типа {} всего : {}", type, count);
    }
    return 0;
}