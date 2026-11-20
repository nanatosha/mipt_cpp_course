#include <charconv>
#include <memory>
#include <print>
#include <string>
#include <string_view>
#include "agent.h"
#include "sources.h"
#include "os_source.h"
#include "../kit/include/l3.2/event_source.h"
#include "../kit/include/l3.2/alert_sink.h"
#include "edr_handle.h"
#include "field_traits.h"

namespace nano_edr {
namespace {
struct Options {
    std::string log_path;
    bool is_quiet = false;
    bool use_file = false;
    std::string out;
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
        else if (a == "--out") {
            if (i + 1 >= argc) return false;
            if (!out->out.empty()) return false;
            ++i;
            out->out = argv[i];
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
    try {
        nano_edr::EdrHandle edr(options.log_path);

        const std::string text = edr.Config("dedup_period_ms", "10000");
        uint64_t dedup_period_ms = 0;
        if (!nano_edr::FieldTraits<uint64_t>::Parse(text, &dedup_period_ms)) {
            std::print(stderr, "[WARN] dedup_period_ms не разбирается: {}\n", text);
            dedup_period_ms = 10000;
        }

        nano_edr::Agent agent(options.is_quiet, dedup_period_ms);


        std::unique_ptr<IEventSource> source;
        if (options.use_file) {
            source = std::make_unique<FileSource>(options.log_path);
        } else {
            source = std::make_unique<OsSource>(options.log_path);
        }
        nano_edr::FanOutSink targets;

        targets.Add(nano_edr::EdrSink(&edr));
        targets.Add(nano_edr::ConsoleSink());
        if (!options.out.empty()) {
            targets.Add(nano_edr::FileSink(options.out));
        }
        agent.Subscribe(std::move(targets));
        source->Run(&agent);
        if (edr.Flush() != EDR_OK) {
            std::print(stderr, "[WARN] детекты не отправлены\n");

        agent.PrintSummary();
        }
    } catch (const std::exception& error) {
        std::print(stderr, "прогон оборван: {}\n", error.what());
        return 1;
    }
    return 0;
}
}  // namespace
}  // namespace nano_edr

int main(int argc, char** argv) {
    nano_edr::Options options;
    if (!nano_edr::ParseArgs(argc, argv, &options)) {
                std::print(stderr,
                   "использование: nano-edr [--file] [--quiet] [--out <файл>] "
                   "<журнал>\n");
        return 2;
    }
    return nano_edr::RunLog(options);
}