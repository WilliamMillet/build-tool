#include "rule_runner.hpp"

#include "errors/error.hpp"

RuleRunner::RuleRunner(RuleGraph rule_graph, Config config)
    : graph(std::move(rule_graph)), cfg(config) {};

void RuleRunner::run_rule(const std::string& rule_name) const {
    if (!graph.is_rule(rule_name)) {
        throw LogicError("Cannot find rule '" + rule_name + "'");
    }

    Visited visited;
    run_rule_recurse(rule_name, visited);
}

void RuleRunner::run_rule_recurse(const std::string& rule_name, Visited& visited) const try {
    // We can only build recipes with commands so we ignore others. Also skip visited
    if (!graph.is_rule(rule_name) || visited.contains(rule_name)) return;

    for (const std::string& dep : graph.dependencies(rule_name)) {
        run_rule(dep);
    }

    visited.insert(rule_name);

    const Rule& rule = graph.get_rule(rule_name);
    if (rule.should_run()) {
        rule.run(cfg);
    }
} catch (std::exception& excep) {
    if (graph.is_rule(rule_name)) {
        Error::update_and_throw(excep, "Running rule '" + rule_name + "'",
                                graph.get_rule(rule_name).get_loc());
    } else {
        Error::update_and_throw(excep, "Running rule '" + rule_name + "'");
    }
}