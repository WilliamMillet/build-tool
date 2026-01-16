#ifndef RULES_H
#define RULES_H

#include <memory>
// #include <stdexcept>
#include <string>
#include <vector>

#include "value.hpp"

enum class RuleType { SINGLE, MULTI, CLEAN };

enum class Step { COMPILE, LINK };

struct QualifiedObj {
    std::string name;

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

struct SingleRule : Rule {
    std::string output;
    std::vector<std::string> dependencies;
    Step step;

    // Rule(std::string rule_name, Value deps, Value step) {
    //     Value::assert_types({{deps, ValueType::LIST}, {step, ValueType::ENUM}});

    //     for (const std::unique_ptr<Value>& v : deps.get<ValueList>().elements) {
    //         Value::assert_types({{*v, ValueType::STRING}});
    //         dependencies.push_back(v->get<std::string>());
    //     }
    // };
};

struct MultiRule : Rule {
    std::vector<std::string> ouput;
};

struct CleanRule : Rule {
    std::vector<std::string> targets;
};

#endif