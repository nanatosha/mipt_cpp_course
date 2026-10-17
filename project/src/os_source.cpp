#include "os_source.h"
#include <stdexcept>

namespace nano_edr {

OsSource::OsSource(const std::string& config_path) 
    : handle_(config_path) {}

void OsSource::Run(Agent* agent) {
  handle_.Subscribe(Agent::Trampoline, agent);
  handle_.Start();
  
  while (true) {
    os_status status = handle_.Wait(1000);
    if (status == OS_OK) {
      break;  
    }
    if (status != OS_TIMEOUT) {
      throw std::runtime_error("ошибка при ожидании событий: " + 
                             std::string(os_status_str(status)));
    }
    
  }
  
  handle_.Stop();
}

}  