#include <stdexcept>
#include <string>
#include <print>
#include <fstream>

#include "../kit/include/l3.2/alert_sink.h"
#include "dedup_sink.h"
#include "../kit/include/l3.2/detection.h"

namespace nano_edr {

// --- ConsoleSink -----------------------------------------------------------

void ConsoleSink::operator()(const Detection& detection) const {
    std::println("{}", ToString(detection));
}

// --- FileSink --------------------------------------------------------------

FileSink::FileSink(const std::string& path) : out_(path) {
    if (!out_.is_open()) {
        throw std::runtime_error("не удалось открыть файл: " + path);
    }
}

void FileSink::operator()(const Detection& detection) {
    out_ << ToString(detection) << '\n';
}

// --- CountingSink ----------------------------------------------------------

void CountingSink::operator()(const Detection& detection) const {
    if (counter_ != nullptr){
        counter_->total++;
    }
    counter_->seen.push_back(detection);

}
// --- FanOutSink ------------------------------------------------------------

void FanOutSink::Add(AlertSink sink_) {
    sinks_.push_back(std::move(sink_));
}

void FanOutSink::operator()(const Detection& detection) const {
    for (const auto& sink : sinks_) {
        sink(detection); 
    }
}
// --- DedupSink -------------------------------------------------------------

DedupSink::DedupSink(AlertSink sink, uint64_t time, std::size_t* sup): inner_(sink), period_ms_(time), suppressed_(sup){

}

void DedupSink::operator()(const Detection& detection) {
    bool f = false;
    if (period_ms_ == 0) {
        inner_(detection);
        return;
    }
    for (int i = 0; i < writes_.size(); ++i){
        if (writes_[i].rule == detection.rule && writes_[i].pid == detection.pid){
            f = true;
            if (detection.ts.ms - writes_[i].ts <= period_ms_){
                if(suppressed_ != nullptr){
                    (*suppressed_)++;
                }
                return;
            }
            else{
                inner_(detection);
                writes_[i].ts = detection.ts.ms;
                return;
            }
        }
    }
    if (!f){
        inner_(detection);
        Memory write;
        write.rule = detection.rule;
        write.pid = detection.pid;
        write.ts = detection.ts.ms;
        writes_.push_back(write);
    }
}

}  // namespace nano_edr
