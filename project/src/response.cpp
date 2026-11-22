#include <string>
#include <print>

#include "../kit/include/l3.3/detection.h"
#include "../kit/include/l3.3/entity_model.h"
#include "../kit/include/l3.3/event.h"
#include "../kit/include/l3.3/os_handle.h"
#include "../kit/include/l3.3/response.h"
#include "text.h"
#include "../kit/include/l3.3/fields.h"
namespace nano_edr {


namespace{
bool system_file(const std::string& s){
    return NormalizePath(s).starts_with("c:\\windows\\") || NormalizePath(s).starts_with("c:\\program files");
}    
}

// --- ResponsePlanner -------------------------------------------------------

ResponsePlanner::ResponsePlanner(const EntityModel* model,
                                 std::string quarantine_dir)
    : model_(model), quarantine_dir_(quarantine_dir) {}

ResponsePlan ResponsePlanner::Plan(const Detection& detection) const {
    ResponsePlan plan;
    // процесс детекта и все его потомки; процесс детекта стоит первым
    const std::vector<std::string> tree =
        model_->GetProcessTree(detection.pid);
    for (const std::string& name : detection.actions) {
        if (name == "kill_process") {
            for (std::size_t i = tree.size(); i > 0; --i) {
                const std::string& pid = tree[i - 1];
                Action action;
                action.kind = ActionKind::kKillProcess;
                action.pid = pid;
                // запись есть у каждого процесса из tree;
                // start — ноль, если запуска процесса модель не видела
                action.expected_start = model_->FindProcess(pid)->start;
                action.reason = detection.rule;
                if (pid != detection.pid) {
                    action.reason += ", потомок " + detection.pid;
                }
                plan.Add(std::move(action));
            }
        } 
        else if (name == "quarantine_source") {
            Action action;
            action.kind = ActionKind::kQuarantineFile;
            if(model_->FindSpawnSource(detection.pid) != nullptr){
                action.path = model_->FindSpawnSource(detection.pid)->path;
                if(!system_file(action.path)){
                    action.to = QuarantinePath(action.path, detection.pid);
                    if (action.to != ""){
                        action.reason = detection.rule;
                        action.reason += ", источник запуска";
                        plan.Add(std::move(action));
                    }
                }
            }
            
        }
        else if (name == "quarantine_image") {
            Action action;
            action.kind = ActionKind::kQuarantineFile;
            if (model_->FindProcess(detection.pid) != nullptr){
                action.path = model_->FindProcess(detection.pid)->image;
                if(!system_file(action.path)){
                    action.to = QuarantinePath(action.path, detection.pid);
                    if (action.to != ""){
                        action.reason = detection.rule;
                        action.reason += ", образ процесса";
                        plan.Add(std::move(action));
                    }
                }
            }
        }
        else if (name == "delete_dropped") {
            for (const std::string& pid : tree) {
                std::vector<std::string> files = model_->GetFilesCreatedBy(pid, true);
                for (const std::string& file_path : files) {
                    if (!system_file(file_path)) {
                        Action action;
                        action.kind = ActionKind::kDeleteFile;
                        action.path = file_path;
                        action.reason = detection.rule + ", принесён " + pid;
                        plan.Add(std::move(action));
                    }
                }
            }
        }
        else if (name == "delete_evidence") {
            for (std::size_t i = 0; i < detection.evidence.size(); ++i) {
                Action action;
                action.kind = ActionKind::kDeleteFile;
                if (detection.evidence[i]->event_type() == EventType::kFileMove){
                    if (FindField(*detection.evidence[i], "to") != nullptr){
                        action.path = *FindField(*detection.evidence[i], "to");
                        if(!system_file(action.path)){
                            action.reason = detection.rule;
                            action.reason += ", файл из детекта";
                            plan.Add(std::move(action));
                        }
                            }
                }
                else{
                    if (FindField(*detection.evidence[i], "path") != nullptr){
                        action.path = *FindField(*detection.evidence[i], "path");
                        if(!system_file(action.path)){
                            action.reason = detection.rule;
                            action.reason += ", файл из детекта";
                            plan.Add(std::move(action));
                        }
                    }
                }
            } 
        // незнакомое имя пропускается
        }
    }
    return plan;

}

std::string ResponsePlanner::QuarantinePath(const std::string& path,
                                            const std::string& pid) const {
    if (quarantine_dir_ == ""){
        return "";
    }

    if (path.find_last_of("\\/") == std::string::npos){
        return std::format("{}\\{}-{}", quarantine_dir_, pid, path);
    }
    return std::format("{}\\{}-{}", quarantine_dir_, pid, path.substr(path.find_last_of("\\/") + 1));
}

// --- Responder -------------------------------------------------------------

Responder::Responder(OsHandle* os, bool dry_run) : os_(os), dry_run_(dry_run) {}

void Responder::Execute(const ResponsePlan& plan) {
    for (const Action& action : plan.actions()) {
        ++stats_.planned;

        // цель, по которой уже отработали: на одном процессе срабатывает
        // несколько правил
        bool repeat = false;
        for (const std::string& target : done_) {
            if (target == action.target()) {
                repeat = true;
                break;
            }
        }
        if (repeat) {
            ++stats_.skipped_repeat;
            continue;
        }
        done_.push_back(action.target());

        if (dry_run_ || os_ == nullptr) {
            std::print("[PLAN] {}\n", ToString(action));
            continue;
        }
        switch (action.kind) {
            case ActionKind::kKillProcess:
                Kill(action);
                break;
            case ActionKind::kQuarantineFile:
                MoveToQuarantine(action);
                break;
            case ActionKind::kDeleteFile:
                Delete(action);
                break;
        }
    }
}
}  // namespace nano_edr
