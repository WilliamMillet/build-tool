#include "rule_graph.hpp"

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "dictionaries/rules.hpp"
#include "errors/error.hpp"

RuleGraph::RuleGraph(std::vector<std::unique_ptr<Rule>> rules) try {
    for (std::unique_ptr<Rule>& rule : rules) {
        const std::string name = rule->get_name();
        for (const std::string& dep : rule->get_deps()) {
            dep_map[name].push_back(dep);
        }
        name_to_rule[name] = std::move(rule);
    }
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Creating rule graph");
}

bool RuleGraph::cyclical_dep_exists() const try {
    // Topological sort used for cycle detection. Operates on the subset of the graph where only
    // recipe nodes remain (no files)
    std::unordered_map<std::string, int> indegree;
    for (const auto& [rule, deps] : dep_map) {
        indegree[rule] = 0;
        for (const std::string& d : deps) {
            if (is_rule(d)) {
                indegree[d] = 0;
            }
        }
    }

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

    size_t reached = 0;
    while (!q.empty()) {
        std::string v = q.front();
        q.pop_front();

        reached++;

        for (const std::string& w : dependencies(v)) {
            if (!is_rule(w)) continue;

            indegree.at(w)--;
            if (indegree.at(w) == 0) {
                q.push_back(w);
            }
        }
    }

    // If we were not able to reach all rules there was a cycle
    return (reached != num_rules());
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Scanning for cyclical dependency in rules");
}

const std::vector<std::string>& RuleGraph::dependencies(const std::string& target) const try {
    auto itm = dep_map.find(target);
    if (itm == dep_map.end()) {
        throw LogicError("Cannot find Rule '" + target + "' on rule graph");
    }
    return itm->second;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Searching for rule dependency");
}

bool RuleGraph::is_rule(const std::string& rule) const { return name_to_rule.contains(rule); }

size_t RuleGraph::num_rules() const { return name_to_rule.size(); }

const Rule& RuleGraph::get_rule(const std::string& name) const { return *name_to_rule.at(name); }