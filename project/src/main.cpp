#include <charconv>
#include <memory>
#include <print>
#include <string>
#include <string_view>
#include "agent.h"
#include "sources.h"
#include "os_source.h"
#include "../kit/include/l3.1/event_source.h"

namespace nano_edr {
namespace {
struct Options {
    std::string log_path;
    bool is_quiet = false;
    bool use_file = false;
};

bool ParseArgs(int argc, char** argv, Options* out) {
    if (argc < 2) {
        return false;
    }
    for (int i = 1; i < argc; ++i) {
        std::string_view a = argv[i];
        if (a == "--quiet") {
            out->is_quiet = true;
        }
        else if (a == "--file") {
            out->use_file = true;
            if (i + 1 >= argc) return false;
            if (!out->log_path.empty()) return false;
            ++i;
            out->log_path = argv[i];
        }
        
        else if (a.starts_with("--")) {
            return false;
        }
        else {
            if (!out->log_path.empty()) return false;
            out->log_path = std::string(a);
        }
    }
    return !out->log_path.empty();
}

int RunLog(const Options& options) {
    Agent agent(options.is_quiet);
    try {
        std::unique_ptr<IEventSource> source;
        if (options.use_file) {
            source = std::make_unique<FileSource>(options.log_path);
        } else {
            source = std::make_unique<OsSource>(options.log_path);
        }
        source->Run(&agent);
    } catch (const std::exception& error) {
        std::print(stderr, "прогон оборван: {}\n", error.what());
        return 1;
    }
    agent.PrintSummary();
    return 0;
}
}  // namespace
}  // namespace nano_edr

int main(int argc, char** argv) {
    nano_edr::Options options;
    if (!nano_edr::ParseArgs(argc, argv, &options)) {
        std::print(stderr,
                   "использование: nano-edr [--file] [--quiet] "
                   "<журнал>\n");
        return 2;
    }
    return nano_edr::RunLog(options);
}