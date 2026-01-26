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
     */
    std::unique_ptr<Rule> make_rule(std::string name, Value obj, Location loc,
                                    VarCategory cat) const;

   private:
    std::unique_ptr<SingleRule> make_single_rule(std::string name, Value obj, Location loc) const;

    std::unique_ptr<MultiRule> make_multi_rule(std::string name, Value obj, Location loc) const;

    std::unique_ptr<CleanRule> make_clean_rule(std::string name, Value obj, Location loc) const;
};

#endif