#include "variable_evaluator.hpp"

#include <deque>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

#include "built_in/func_registry.hpp"
#include "dictionaries/config.hpp"
#include "dictionaries/rules.hpp"
#include "errors/error.hpp"
#include "parsing/parser.hpp"
#include "value.hpp"

VariableEvaluator::VariableEvaluator(std::vector<ParsedVariable> vars, FuncRegistry _fn_reg)
    : raw_vars(std::move(vars)), fn_reg(_fn_reg) {};

QualifiedDicts VariableEvaluator::evaluate() try {
    std::unordered_map<std::string, std::vector<std::string>> dep_graph;
    for (const ParsedVariable& v : raw_vars) {
        dep_graph[v.identifier] = aggregate_deps(v);
    }

    sort_by_eval_order(raw_vars, dep_graph);

    std::vector<std::unique_ptr<Rule>> rules;
    std::unique_ptr<Config> cfg;

    for (const ParsedVariable& var : raw_vars) {
        var_map[var.identifier] = var.expr->evaluate(var_map, fn_reg);
        process_val(var, var_map.at(var.identifier), rules, cfg);
    }

    if (cfg == nullptr) {
        throw LogicError("Could not find <Config> qualified dictionary. Config must be added");
    }

    return QualifiedDicts{std::move(rules), *cfg};
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Variable evaluation (includes all dictionaries)");
}

std::vector<std::string> VariableEvaluator::aggregate_deps(const ParsedVariable& var) const {
    // Expr is an AST with no cycles so we do a level order traversal
    std::deque<Expr*> q = {var.expr.get()};
    std::vector<std::string> deps;

    while (!q.empty()) {
        Expr* v = q.front();
        q.pop_front();

        VarRefExpr* v_var = dynamic_cast<VarRefExpr*>(v);
        if (v_var != nullptr) {
            deps.push_back(v_var->identifier);
        }

        for (Expr* child : v->get_children()) {
            q.push_back(child);
        }
    }

    return deps;
}

void VariableEvaluator::sort_by_eval_order(std::vector<ParsedVariable>& vars,
                                           const DepGraph& dep_graph) const try {
    std::unordered_map<std::string, ParsedVariable> var_id_map;
    std::unordered_map<std::string, int> indegree;

    for (ParsedVariable& v : vars) {
        indegree[v.identifier] = 0;
        var_id_map[v.identifier] = std::move(v);
    }

    for (const auto& [id, adj] : dep_graph) {
        for (const std::string& dep : adj) {
            indegree[dep]++;
        }
    }

    std::deque<std::string> q;
    for (const auto& [id, indeg] : indegree) {
        if (indeg == 0) {
            q.push_back((id));
        };
    }

    std::vector<std::string> ordered;
    while (!q.empty()) {
        std::string v = q.front();
        q.pop_front();

        ordered.push_back(v);

        for (const std::string& dep : dep_graph.at(v)) {
            indegree.at(dep)--;
            if (indegree[dep] == 0) {
                q.push_back(dep);
            }
        }
    }

    if (ordered.size() != dep_graph.size()) {
        throw LogicError("Cyclical dependency between variables detected. Variable count was " +
                         std::to_string(dep_graph.size()) +
                         " but topological traversal could only reach " +
                         std::to_string(ordered.size()));
    }

    vars.clear();
    for (const std::string& id : std::views::reverse(ordered)) {
        vars.push_back(std::move(var_id_map.at(id)));
    }
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Determining variable evaluation order");
}

void VariableEvaluator::process_val(const ParsedVariable& var, Value& val,
                                    std::vector<std::unique_ptr<Rule>>& rules,
                                    std::unique_ptr<Config>& cfg) {
    const std::string& id = var.identifier;

    switch (var.category) {
        case VarCategory::CLEAN: {
            rules.push_back(std::make_unique<CleanRule>(std::move(var.identifier), std::move(val),
                                                        std::move(var.loc)));
            break;
        }
        case VarCategory::SINGLE_RULE: {
            rules.push_back(std::make_unique<SingleRule>(std::move(id), std::move(val), var.loc));
            break;
        }
        case VarCategory::MULTI_RULE: {
            rules.push_back(std::make_unique<MultiRule>(std::move(id), std::move(val), var.loc));
            break;
        }
        case VarCategory::CONFIG: {
            if (cfg != nullptr) {
                throw SyntaxError(
                    "Duplicate <Config> dictionaries detected. Only one configuration may be set",
                    var.loc);
            }
            cfg = std::make_unique<Config>(std::move(id), std::move(val));
            break;
        }
        case VarCategory::REGULAR: {
            // Not a qualified dictionary so we skip
            break;
        };
    }
}