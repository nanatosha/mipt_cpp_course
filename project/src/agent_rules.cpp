#include "agent_rules.h"

namespace nano_edr {

std::vector<MatchRule> BuildAgentRules() {
  std::vector<MatchRule> rules;

  // 1. script_host_from_temp (high)
  {
    MatchRule rule("script_host_from_temp", Severity::kHigh);
    rule.AddCondition(Condition::Equals("type", {"process_start"}));
    rule.AddCondition(
        Condition::EndsWith("image", {"wscript.exe", "cscript.exe"}));
    rule.AddCondition(Condition::Contains("cmdline", {
        "\\appdata\\local\\temp\\", "/appdata/local/temp/",
        "\\windows\\temp\\", "/windows/temp/",
        "%temp%", "%tmp%"}));
    rule.AddAction("kill_process");
    rules.push_back(std::move(rule));
  }

  // 2. lolbin_download (high)
  {
    MatchRule rule("lolbin_download", Severity::kHigh);
    rule.AddCondition(Condition::Equals("type", {"process_start"}));
    rule.AddCondition(
        Condition::EndsWith("image", {"certutil.exe", "bitsadmin.exe"}));
    rule.AddCondition(Condition::Contains("cmdline", {
        "urlcache", "transfer", "http:", "https:"}));
    rule.AddAction("kill_process");
    rules.push_back(std::move(rule));
  }

  // 3. hidden_powershell (medium)
  {
    MatchRule rule("hidden_powershell", Severity::kMedium);
    rule.AddCondition(Condition::Equals("type", {"process_start"}));
    rule.AddCondition(
        Condition::EndsWith("image", {"powershell.exe", "pwsh.exe"}));
    rule.AddCondition(Condition::Contains("cmdline", {
        "-w hidden", "-windowstyle hidden", "-enc"}));
    rule.AddAction("kill_process");
    rules.push_back(std::move(rule));
  }

  // 4. autostart_write по path (high)
  {
    MatchRule rule("autostart_write", Severity::kHigh);
    rule.AddCondition(
        Condition::Equals("type", {"file_create", "file_write"}));
    rule.AddCondition(Condition::Contains("path", {
        "\\start menu\\programs\\startup\\",
        "/start menu/programs/startup/"}));
    rule.AddAction("quarantine_file");
    rules.push_back(std::move(rule));
  }

  // 5. autostart_write по to (high)
  {
    MatchRule rule("autostart_write", Severity::kHigh);
    rule.AddCondition(Condition::Equals("type", {"file_move"}));
    rule.AddCondition(Condition::Contains("to", {
        "\\start menu\\programs\\startup\\",
        "/start menu/programs/startup/"}));
    rule.AddAction("quarantine_file");
    rules.push_back(std::move(rule));
  }

  // 6. ransom_extension по path (critical)
  {
    MatchRule rule("ransom_extension", Severity::kCritical);
    rule.AddCondition(
        Condition::Equals("type", {"file_create", "file_write"}));
    rule.AddCondition(Condition::EndsWith("path", {".locked"}));
    rule.AddAction("quarantine_file");
    rules.push_back(std::move(rule));
  }

  // 7. ransom_extension по to (critical)
  {
    MatchRule rule("ransom_extension", Severity::kCritical);
    rule.AddCondition(Condition::Equals("type", {"file_move"}));
    rule.AddCondition(Condition::EndsWith("to", {".locked"}));
    rule.AddAction("quarantine_file");
    rules.push_back(std::move(rule));
  }

  return rules;
}

}  // namespace nano_edr