#include "agent_rules.h"
#include "../kit/include/l3.3/conditions.h"
#include "../kit/include/l3.3/rule.h"
#include "make_rule.h"
#include "../kit/include/l3.3/combinators.h"
#include "../kit/include/l3.3/entity_model.h"


namespace nano_edr {

RuleEngine BuildAgentRules(const EntityModel* model){
    RuleEngine engine;

    // 
    {
        auto rule = std::make_unique<MatchRule>("script_host_from_temp", Severity::kHigh);
        rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
        
        auto image_cond = std::make_unique<AnyOf>();
        image_cond->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
        image_cond->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
        rule->AddCondition(std::move(image_cond));

        auto cmdline_cond = std::make_unique<AnyOf>();
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "\\appdata\\local\\temp\\"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "/appdata/local/temp/"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "\\windows\\temp\\"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
        rule->AddCondition(std::move(cmdline_cond));

        rule->AddAction("kill_process");
        rule->AddAction("quarantine_source");    
        rule->AddAction("delete_dropped");     
        engine.AddRule(std::move(rule));
    }

    // 
    {
        auto rule = std::make_unique<MatchRule>("lolbin_download", Severity::kHigh);
        rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
        
        auto image_cond = std::make_unique<AnyOf>();
        image_cond->Add(std::make_unique<FieldEndsWith>("image", "certutil.exe"));
        image_cond->Add(std::make_unique<FieldEndsWith>("image", "bitsadmin.exe"));
        rule->AddCondition(std::move(image_cond));

        auto cmdline_cond = std::make_unique<AnyOf>();
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "urlcache"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "transfer"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "http:"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "https:"));
        rule->AddCondition(std::move(cmdline_cond));

        rule->AddAction("kill_process");
        engine.AddRule(std::move(rule));
    }

    // 
    {
        auto rule = std::make_unique<MatchRule>("hidden_powershell", Severity::kMedium);
        rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
        
        auto image_cond = std::make_unique<AnyOf>();
        image_cond->Add(std::make_unique<FieldEndsWith>("image", "powershell.exe"));
        image_cond->Add(std::make_unique<FieldEndsWith>("image", "pwsh.exe"));
        rule->AddCondition(std::move(image_cond));

        auto cmdline_cond = std::make_unique<AnyOf>();
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "-w hidden"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "-windowstyle hidden"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "-enc"));
        rule->AddCondition(std::move(cmdline_cond));

        rule->AddAction("kill_process");
        engine.AddRule(std::move(rule));
    }

    // 
    {
        auto rule = std::make_unique<MatchRule>("autostart_write", Severity::kHigh);
        rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{
            EventType::kFileCreate, EventType::kFileWrite, EventType::kFileMove
        }));
        
        auto path_cond = std::make_unique<AnyOf>();
        path_cond->Add(std::make_unique<FieldContains>("path", "\\start menu\\programs\\startup\\"));
        path_cond->Add(std::make_unique<FieldContains>("path", "/start menu/programs/startup/"));
        path_cond->Add(std::make_unique<FieldContains>("to", "\\start menu\\programs\\startup\\"));
        path_cond->Add(std::make_unique<FieldContains>("to", "/start menu/programs/startup/"));
        rule->AddCondition(std::move(path_cond));

                rule->AddAction("delete_evidence");     
        engine.AddRule(std::move(rule));
    }

    // 
    {
        auto rule = std::make_unique<MatchRule>("ransom_extension", Severity::kCritical);
        rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{
            EventType::kFileCreate, EventType::kFileWrite, EventType::kFileMove
        }));
        
        auto path_cond = std::make_unique<AnyOf>();
        path_cond->Add(std::make_unique<FieldEndsWith>("path", ".locked"));
        path_cond->Add(std::make_unique<FieldEndsWith>("to", ".locked"));
        rule->AddCondition(std::move(path_cond));

        rule->AddAction("kill_process");          
        rule->AddAction("quarantine_image");      
        engine.AddRule(std::move(rule));
    }

    // 
    {
        auto beacon = std::make_unique<SequenceRule>("script_host_beacon", Severity::kCritical, 10000);
        
        auto start = std::make_unique<AllOf>();
        start->Add(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
        
        auto image_cond = std::make_unique<AnyOf>();
        image_cond->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
        image_cond->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
        start->Add(std::move(image_cond));

        auto cmdline_cond = std::make_unique<AnyOf>();
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "\\appdata\\local\\temp\\"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "/appdata/local/temp/"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "\\windows\\temp\\"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
        cmdline_cond->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
        start->Add(std::move(cmdline_cond));

        beacon->SetFirst(std::move(start));
        beacon->SetSecond(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kNetConnect}));
        beacon->AddAction("kill_process");
        engine.AddRule(std::move(beacon));
    }
    {
        {
    auto mass_write = MakeRule<ThresholdRule<128>>(
        "ransomware_mass_write", Severity::kCritical,
        20,      // сколько событий
        10000,   // за сколько миллисекунд
        "pid");  // считать отдельно для каждого pid
    mass_write->SetCondition(MakeCondition(AllOfStatic(
    EventTypeIs({EventType::kFileCreate, EventType::kFileWrite,
                 EventType::kFileMove}),
    AnyOfStatic(FieldContains("path", "\\Documents\\"),
                FieldContains("path", "\\Desktop\\"),
    FieldContains("to", "\\Documents\\"), FieldContains("to", "\\Desktop\\")))));
    mass_write->AddAction("kill_process");
    mass_write->AddAction("quarantine_image"); 
    engine.AddRule(std::move(mass_write));
}
        
}
{
        
    {
        auto shell = MakeRule<SequenceRule>("shell_spawns_script_host", Severity::kHigh, 30000);
        
        
        auto first_cond = std::make_unique<AllOf>();
        first_cond->Add(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
        
        auto image_first = std::make_unique<AnyOf>();
        image_first->Add(std::make_unique<FieldEndsWith>("image", "cmd.exe"));
        image_first->Add(std::make_unique<FieldEndsWith>("image", "powershell.exe"));
        image_first->Add(std::make_unique<FieldEndsWith>("image", "pwsh.exe"));
        first_cond->Add(std::move(image_first));
        
        shell->SetFirst(std::move(first_cond));

        
        auto second_cond = std::make_unique<AllOf>();
        second_cond->Add(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
        
        auto image_second = std::make_unique<AnyOf>();
        image_second->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
        image_second->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
        second_cond->Add(std::move(image_second));
        
        shell->SetSecond(std::move(second_cond));

        
        shell->set_child_of_prev(true);
        shell->SetModel(model);
        shell->AddAction("kill_process");
        
        engine.AddRule(std::move(shell));
    }
}
{
    auto dropped = MakeRule<LambdaRule>("spawned_from_dropped_file", Severity::kHigh,
    [model](const Event& event) {
        if (event.event_type() != EventType::kProcessStart) {
            return false;
        }
        const FileRecord* source = model->FindSpawnSource(event.pid());
        if(source != nullptr && source->writer_pid != "" && source->writer_pid != event.pid()){
            return true;
        }
        else{
            return false;
        }
    });
    
    dropped->AddAction("kill_process");
    dropped->AddAction("quarantine_source");      
    dropped->AddAction("delete_dropped");        
    engine.AddRule(std::move(dropped));
}

    return engine;
}

}  // namespace nano_edr