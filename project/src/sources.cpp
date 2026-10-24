#include "sources.h"

#include <fstream>
#include <stdexcept>

#include "../kit/include/l2.2/parse.h"

namespace nano_edr {

FileSource::FileSource(const std::string& path) : path_(path) {}



void FileSource::Run(Agent* agent) {
    std::ifstream log(path_);
    if (!log) {
        throw std::runtime_error("не удалось открыть журнал: " + path_);
    }

    std::string line;
    while (std::getline(log, line)) {
        if (IsBlankOrComment(line)) {
            continue;
        }

        EventParts parts;
        if (!ParseEventParts(line, &parts)) {
            agent->ReportBadLine();
            continue;
        }

        try {
            Event event(parts);
            agent->HandleEvent(event);
        } catch (const std::exception&) {
            agent->ReportDropped();
        }
    }
}

}