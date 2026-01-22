#include "rule_runner.hpp"

#include "errors/error.hpp"

RuleRunner::RuleRunner(std::shared_ptr<const RuleGraph> rule_graph,
                       std::shared_ptr<const Config> cfg,
                       std::shared_ptr<ProcessSpawner> proc_spawner,
                       std::shared_ptr<FSGateway> fs_gw)
    : graph(rule_graph), config(cfg), process_runner(proc_spawner), fs_gateway(fs_gw) {};

void RuleRunner::run_rule(const std::string& rule_name) const {
    if (!graph->is_rule(rule_name)) {
        throw LogicError("Cannot find rule '" + rule_name + "'");
    }

    Visited visited;
    run_rule_recurse(rule_name, visited);
}

void RuleRunner::run_rule_recurse(const std::string& rule_name, Visited& visited) const try {
    // We can only build recipes with commands so we ignore others. Also skip visited
    if (!graph->is_rule(rule_name) || visited.contains(rule_name)) return;

    for (const std::string& dep : graph->dependencies(rule_name)) {
        run_rule_recurse(dep, visited);
    }

    visited.insert(rule_name);

    const Rule& rule = graph->get_rule(rule_name);
    if (rule.should_run(*fs_gateway)) {
        for (Command& cmd : rule.get_commands(*config)) {
            process_runner->run(cmd);
        }
    }
} catch (std::exception& excep) {
    if (graph->is_rule(rule_name)) {
        Error::update_and_throw(excep, "Running rule '" + rule_name + "'",
                                graph->get_rule(rule_name).get_loc());
    } else {
        Error::update_and_throw(excep, "Running rule '" + rule_name + "'");
    }
}