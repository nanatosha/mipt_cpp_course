#ifndef NANO_EDR_SOURCES_H
#define NANO_EDR_SOURCES_H

#include <string>

#include "../kit/include/l2.3/event_source.h"

namespace nano_edr {

class FileSource : public IEventSource {
 public:
    explicit FileSource(const std::string& path);

    void Run(IEventHandler* handler) override;

 private:
    std::string path_;
};

}  // namespace nano_edr

#endif