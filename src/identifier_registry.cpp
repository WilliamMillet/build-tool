#include "identifier_registry.hpp"

#include <deque>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "built_in/func_registry.hpp"
#include "parsing/parser.hpp"
#include "value.hpp"

IdentifierRegistry::IdentifierRegistry(std::vector<ParsedVariable> vars, FuncRegistry fn_reg_)
    : fn_reg(fn_reg_) {
    std::unordered_map<std::string, std::vector<std::string>> dep_graph;
    for (const ParsedVariable& v : vars) {
        dep_graph[v.identifier] = aggregate_deps(v);
    }

    sort_by_eval_order(vars, dep_graph);

    for (const ParsedVariable& var : vars) {
        var_map[var.identifier] = var.expr->evaluate(var_map, fn_reg_);
    }
}

Value IdentifierRegistry::resolve_var(std::string& identifier) const {
    auto var_itm = var_map.find(identifier);
    if (var_itm == var_map.end()) {
        throw std::invalid_argument("Cannot resolve variable name '" + identifier + "'");
    }

    return var_itm->second;
}

std::vector<std::string> IdentifierRegistry::aggregate_deps(const ParsedVariable& var) const {
    // Expr is an AST with no cycles so we do a level order traversal
    std::deque<Expr*> q = {var.expr.get()};
    std::vector<std::string> deps;

    while (q.empty()) {
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

void IdentifierRegistry::sort_by_eval_order(std::vector<ParsedVariable>& vars,
                                            const DepGraph& dep_graph) const {
    std::unordered_map<std::string, ParsedVariable> var_id_map;
    for (ParsedVariable& v : vars) {
        var_id_map[v.identifier] = std::move(v);
    }

    std::unordered_map<std::string, int> indegree;
    for (const auto& [id, adj] : dep_graph) {
        for (const std::string& dep : adj) {
            indegree[dep]++;
        }
    }

    std::deque<std::string> q;
    for (const auto& [id, indeg] : indegree) {
        if (indeg == 0) q.push_back((id));
    }

    std::vector<ParsedVariable> ordered;
    while (!q.empty()) {
        std::string v = q.front();
        q.pop_front();

        ordered.push_back(std::move(var_id_map.at(v)));

        for (const std::string& dep : dep_graph.at(v)) {
            indegree.at(dep)--;
            if (indegree[dep] == 0) {
                q.push_back(dep);
            }
        }
    }

    if (ordered.size() != dep_graph.size()) {
        throw std::invalid_argument("Cyclical dependency between variables");
    }
}