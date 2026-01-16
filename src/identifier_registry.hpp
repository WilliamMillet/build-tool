#ifndef VAR_EVALUATOR_H
#define VAR_EVALUATOR_H

#include <string>
#include <unordered_map>
#include <vector>

#include "built_in/func_registry.hpp"
#include "parsing/parser.hpp"
#include "value.hpp"

using DepGraph = std::unordered_map<std::string, std::vector<std::string>>;
using VarMap = std::unordered_map<std::string, Value>;

class IdentifierRegistry {
   public:
    /**
     * Create an identifier registry by evaluating variables in the topological order
     * @param vars The parsed, but not evaluated variables
     * @param fn_reg The registry of functions used in evaluating variables
     */
    IdentifierRegistry(std::vector<ParsedVariable> vars, FuncRegistry fn_reg);

    /** Get a variable from it's identifier (including name and namespace) */
    Value resolve_var(std::string& identifier) const;

   private:
    VarMap var_map;
    FuncRegistry fn_reg;

    /** Given an expression, return its dependencies */
    std::vector<std::string> aggregate_deps(const ParsedVariable& var) const;

    /**
     * Topologically sort each expression such that it can be evaluated in the correct order.
     * The vars vector passed will be the one edited
     */
    void sort_by_eval_order(std::vector<ParsedVariable>& vars, const DepGraph& dep_graph) const;
};

#endif
