#ifndef NANO_EDR_OS_SOURCE_H
#define NANO_EDR_OS_SOURCE_H
#include <string>
#include "agent.h"
#include "../kit/include/l2.2/os_handle.h"

namespace nano_edr {
class OsSource {
public:
  explicit OsSource(const std::string& config_path);
  void Run(Agent* agent);

private:
  OsHandle handle_;
};
}  // namespace nano_edr
#endif