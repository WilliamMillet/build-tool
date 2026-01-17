#include "rule_graph.hpp"

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include "dictionaries/rules.hpp"

RuleGraph::RuleGraph(std::vector<Rule> rules) {
    for (Rule& rule : rules) {
        std::string name = rule.get_name();
        name_to_rule[name] = std::move(rule);
        for (const std::string& dep : rule.get_deps()) {
            dep_map[name].push_back(dep);
        }
    }
}

bool RuleGraph::cyclical_dep_exists() const {
    // Topological sort used for cycle detection

    std::unordered_map<std::string, int> indegree;
    for (const auto& [rule, deps] : dep_map) {
        for (const std::string& d : deps) {
            if (is_rule(d)) {
                indegree[d]++;
            }
        }
    }

    std::deque<std::string> q;
    for (const auto& [rule, indeg] : indegree) {
        if (indeg == 0) {
            q.push_back(rule);
        }
    }

    int reached = 0;
    while (!q.empty()) {
        std::string v = q.front();
        q.pop_front();

        reached++;

        for (const std::string& w : dependencies(v)) {
            indegree.at(w)--;
            if (indegree.at(w) == 0) {
                q.push_back(w);
            }
        }
    }

    // If we were not able to reach all rules there was a cycle
    return reached = num_rules();
}

const std::vector<std::string>& RuleGraph::dependencies(const std::string& target) const {
    auto itm = dep_map.find(target);
    if (itm == dep_map.end()) {
        throw std::invalid_argument("Cannot find Rule '" + target + "' on rule graph");
    }
    return itm->second;
}

bool RuleGraph::is_rule(const std::string& rule) const { return name_to_rule.contains(rule); }

size_t RuleGraph::num_rules() const { return name_to_rule.size(); }

const Rule& RuleGraph::get_rule(const std::string& name) const { return name_to_rule.at(name); }