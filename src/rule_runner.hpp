#ifndef RULE_RUNNER_H
#define RULE_RUNNER_H

#include "rule_graph.hpp"

class RuleRunner {
   public:
    RuleRunner(RuleGraph rule_graph, Config config);

    /** Run a specified user rule */
    void run_rule(const std::string& rule_name) const;

   private:
    RuleGraph graph;
    Config cfg;
};

#endif