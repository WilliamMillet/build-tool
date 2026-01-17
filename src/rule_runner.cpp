#include "rule_runner.hpp"

RuleRunner::RuleRunner(RuleGraph rule_graph, Config config) : graph(rule_graph), cfg(config) {};

void RuleRunner::run_rule(const std::string& rule_name) const {
    // We can only build recipes with commands so we ignore others
    if (!graph.is_rule(rule_name)) return;

    for (const std::string& dep : graph.dependencies(rule_name)) {
        run_rule(dep);
    }

    const Rule& rule = graph.get_rule(rule_name);
    if (rule.should_run()) {
        rule.run(cfg);
    }
}