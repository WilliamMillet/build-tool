#ifndef RULE_RUNNER_H
#define RULE_RUNNER_H

#include <memory>
#include <unordered_set>

#include "dictionaries/config.hpp"
#include "io/fs_gateway.hpp"
#include "io/proc_spawner.hpp"
#include "rule_graph.hpp"

using Visited = std::unordered_set<std::string>;

class RuleRunner {
   public:
    RuleRunner(std::shared_ptr<const RuleGraph> rule_graph, std::shared_ptr<const Config> cfg,
               std::shared_ptr<ProcessSpawner> proc_spawner, std::shared_ptr<FSGateway> fs_gw);

    /** Run a specified user rule */
    void run_rule(const std::string& rule_name) const;

   private:
    std::shared_ptr<const RuleGraph> graph;
    std::shared_ptr<const Config> config;
    std::shared_ptr<ProcessSpawner> process_runner;
    std::shared_ptr<FSGateway> fs_gateway;

    /** Recursive helper for run_rule */
    void run_rule_recurse(const std::string& rule_name, Visited& visited) const;
};

#endif