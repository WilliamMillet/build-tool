#include "rule_runner.hpp"

#include "errors/error.hpp"

RuleRunner::RuleRunner(RuleGraph rule_graph, Config cfg, ProcessSpawner* proc_runner,
                       FSGateway* fs_gw)
    : graph(std::move(rule_graph)),
      config(std::move(cfg)),
      process_runner(std::move(proc_runner)),
      fs_gateway(std::move(fs_gw)) {};

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
        run_rule_recurse(dep, visited);
    }

    visited.insert(rule_name);

    const Rule& rule = graph.get_rule(rule_name);
    if (rule.should_run(*fs_gateway)) {
        for (Command& cmd : rule.get_commands(config)) {
            process_runner->run(cmd);
        }
    }
} catch (std::exception& excep) {
    if (graph.is_rule(rule_name)) {
        Error::update_and_throw(excep, "Running rule '" + rule_name + "'",
                                graph.get_rule(rule_name).get_loc());
    } else {
        Error::update_and_throw(excep, "Running rule '" + rule_name + "'");
    }
}