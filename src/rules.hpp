#ifndef RULES_H
#define RULES_H

#include <memory>
// #include <stdexcept>
#include <string>
#include <vector>

#include "value.hpp"

enum class RuleType { SINGLE, MULTI, CLEAN };

enum class Step { COMPILE, LINK };

struct Rule {
    std::string name;
    std::vector<std::string> dependencies;
    Step step;

    Rule(Value rule_name, Value deps, Value step) {
        Value::assert_types(
            {{rule_name, ValueType::STRING}, {deps, ValueType::LIST}, {step, ValueType::ENUM}});
        name = rule_name.get<std::string>();

        for (const std::unique_ptr<Value>& v : deps.get<ValueList>().elements) {
            Value::assert_types({{*v, ValueType::STRING}});
            dependencies.push_back(v->get<std::string>());
        }
    }

    // /** Convert a rule type string to a RuleType enum */
    // static RuleType str_to_type(std::string& type) {
    //     if (type == "Rule") {
    //         return RuleType::SINGLE;
    //     } else if (type == "MultiRule") {
    //         return RuleType::MULTI;
    //     } else if (type == "Clean") {
    //         return RuleType::CLEAN;
    //     } else {
    //         throw std::invalid_argument("Unknown rule type '<" + type + ">'");
    //     }
    // }
};

struct SingleRule : public Rule {
    std::string output;
};

struct MultiRule : public Rule {
    std::vector<std::string> ouput;
};

struct CleanRule {
    std::vector<std::string> targets;
};

#endif