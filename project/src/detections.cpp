
#include <format>
#include <string>
#include "../kit/include/l3.2/detection.h"

namespace nano_edr{

std::string ToString(const Detection& detection){
    if(detection.chain != ""){
        return std::format("[DETECT] {}  {}  ts={} pid={} chain={}",
                   SeverityName(detection.severity), detection.rule,
                   detection.ts.ms, detection.pid, detection.chain);
    }
    else{
        return std::format("[DETECT] {}  {}  ts={} pid={}",
                   SeverityName(detection.severity), detection.rule,
                   detection.ts.ms, detection.pid);
    }

}

}