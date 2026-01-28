#ifndef RULE_FACTORY_H
#define RULE_FACTORY_H

#include <memory>

#include "../parsing/parser.hpp"
#include "rules.hpp"

class RuleFactory {
   public:
    /**
     * Create a rule object based upon a dictionary value containing a SingleRule, MultiRule rule or
     * clean rule.
     * @param name The name of the rule (also used as the output)
     * @param obj The evaluated dictionary representing the rule
     * @param loc The location the rule was written at in the original file
     * @param cat The category of the variable (regular variable, MutiRule qualified var, etc)
     * @returns A pointer to a polymorphic rule
     * @throws If the rule category is unknown
     */
    std::unique_ptr<Rule> make_rule(std::string name, Value obj, Location loc,
                                    VarCategory cat) const;

   private:
    std::unique_ptr<SingleRule> make_single_rule(std::string name, Value obj, Location loc) const;

    std::unique_ptr<MultiRule> make_multi_rule(std::string name, Value obj, Location loc) const;

    std::unique_ptr<CleanRule> make_clean_rule(std::string name, Value obj, Location loc) const;
};

#endif