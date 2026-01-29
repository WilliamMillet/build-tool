#ifndef VAR_EVALUATOR_H
#define VAR_EVALUATOR_H

#include <string>
#include <unordered_map>
#include <vector>

#include "built_in/func_registry.hpp"
#include "dictionaries/config.hpp"
#include "dictionaries/qualified_dicts.hpp"
#include "dictionaries/rules.hpp"
#include "parsing/parser.hpp"
#include "value.hpp"

using DepGraph = std::unordered_map<std::string, std::vector<std::string>>;
using VarMap = std::unordered_map<std::string, Value>;

class VariableEvaluator {
   public:
    /**
     * @brief Create an identifier registry
     *
     * @param vars The parsed, but not evaluated variables
     * @param _fn_reg The registry of functions used in evaluating variables
     */
    VariableEvaluator(std::vector<ParsedVariable> vars, FuncRegistry _fn_reg);

    /**
     * @brief Evaluating variables in the topological order
     *
     * @return QualifiedDicts the qualified dictionaries extracted from the evaluated variables
     * @throws If no config could be found
     */
    QualifiedDicts evaluate();

   private:
    std::vector<ParsedVariable> raw_vars;

    VarMap var_map;
    FuncRegistry fn_reg;

    /** Given an expression, return its dependencies */
    std::vector<std::string> aggregate_deps(const ParsedVariable& var) const;

    /**
     * Topologically sort each expression such that it can be evaluated in the correct order.
     * The vars vector passed will be the one edited
     */
    void sort_by_eval_order(std::vector<ParsedVariable>& vars, const DepGraph& dep_graph) const;

    /**
     * Given an evaluated value, update the current rules vector and cfg if applicable
     * @param vars The parsed, but not evaluated variable
     * @param val The newly evaluated variable
     * @param rules The existing collection of identified rules
     * @param cfg A pointer to store a config if found
     */
    void process_val(const ParsedVariable& var, Value& val,
                     std::vector<std::unique_ptr<Rule>>& rules, std::unique_ptr<Config>& cfg);
};

#endif
