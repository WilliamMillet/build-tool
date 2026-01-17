#ifndef RULE_GRAPH_H
#define RULE_GRAPH_H

#include <string>
#include <unordered_map>
#include <vector>

#include "dictionaries/config.hpp"
#include "dictionaries/rules.hpp"

class RuleGraph {
   public:
    RuleGraph(std::vector<Rule> rules);

    /** Determine if there is a cyclical dependency amongst rules */
    bool cyclical_dep_exists() const;

    /** Returns a list  */
    const std::vector<std::string>& dependencies(const std::string& target) const;

    /** Returns a list of all string shell commands needed to be executed to run a rule */
    std::vector<std::string> get_build_cmds(const Config& cfg, const std::string& target) const;

    /** Returns true iff a rule exists on the graph */
    bool is_rule(const std::string& rule) const;

    /** Get the number of rules in the graph */
    size_t num_rules() const;

   private:
    std::unordered_map<std::string, Rule> name_to_rule;
    /** Map of rule to there dependencies. No keys exist for files/non-rule dependencies */
    std::unordered_map<std::string, std::vector<std::string>> dep_map;
};

#endif