#ifndef RULE_RUNNER_H
#define RULE_RUNNER_H

#include <unordered_set>

#include "dictionaries/config.hpp"
#include "io/fs_gateway.hpp"
#include "io/proc_spawner.hpp"
#include "rule_graph.hpp"

using Visited = std::unordered_set<std::string>;

class RuleRunner {
   public:
    RuleRunner(RuleGraph rule_graph, Config cfg, ProcessSpawner* proc_spawner, FSGateway* fs_gw);

    /** Run a specified user rule */
    void run_rule(const std::string& rule_name) const;

   private:
    RuleGraph graph;
    Config config;
    ProcessSpawner* process_runner;
    FSGateway* fs_gateway;

    /** Recursive helper for run_rule */
    void run_rule_recurse(const std::string& rule_name, Visited& visited) const;
};

#endif