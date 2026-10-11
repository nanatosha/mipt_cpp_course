#include <charconv>
#include <print>
#include <string>
#include <string_view>
#include "agent.h"
#include "sources.h"
#include "os_source.h"

namespace nano_edr {
namespace {
struct Options {
    std::string log_path;
    bool is_quiet = false;
    bool use_file = false;
    std::size_t window_size = 64;
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
            if (i + 1 >= argc) {
                return false;  
            }
            if (!out->log_path.empty()) {
                return false;  
            }
            ++i;
            out->log_path = argv[i];
        }
        else if (a == "--window-size") {
            if (i + 1 >= argc) {
                return false;
            }
            ++i;
            std::string_view s = argv[i];
            long long n = 0;
            auto parsed = std::from_chars(s.data(), s.data() + s.size(), n);
            if (parsed.ec != std::errc() || parsed.ptr != s.data() + s.size()) {
                return false;
            }
            out->window_size = static_cast<std::size_t>(n);
        } 
        else if (a.starts_with("--")) {
            
            return false;
        }
        else {
            
            if (!out->log_path.empty()) {
                return false;  
            }
            out->log_path = std::string(a);
        }
    }
    
    return !out->log_path.empty();
}

int RunLog(const Options& options) {
    Agent agent(options.window_size, options.is_quiet);
    
    try {
        if (options.use_file) {
            FileSource source(options.log_path);
            source.Run(&agent);
        } else {
            OsSource source(options.log_path);
            source.Run(&agent);
        }
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
                   "[--window-size N] <журнал>\n");
        return 2;
    }
    
    return nano_edr::RunLog(options);
}