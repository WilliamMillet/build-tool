#ifndef RULES_H
#define RULES_H

#include <stdexcept>
#include <string>
#include <vector>

#include "built_in/enums.hpp"
#include "value.hpp"

enum class RuleType { SINGLE, MULTI, CLEAN };

struct Rule {
    std::string name;
};

struct SingleRule : Rule {
    std::vector<std::string> deps;
    Step step;

    SingleRule(std::string _name, Value _deps, Value _step) {
        name = std::move(_name);
        Value::assert_types({{_deps, ValueType::LIST}, {_step, ValueType::ENUM}});
        deps = ValueUtils::vectorise<std::string>(_deps.get<ValueList>(), ValueType::STRING);
        step = resolve_enum<Step>(_step.get<ScopedEnumValue>());
    };
};

struct MultiRule : Rule {
    std::vector<std::string> deps;
    std::vector<std::string> output;
    Step step;

    MultiRule(std::string _name, Value _deps, Value _output, Value _step) {
        name = std::move(_name);
        Value::assert_types(
            {{_deps, ValueType::LIST}, {_output, ValueType::LIST}, {_step, ValueType::ENUM}});
        deps = ValueUtils::vectorise<std::string>(_deps.get<ValueList>(), ValueType::STRING);
        output = ValueUtils::vectorise<std::string>(_output.get<ValueList>(), ValueType::STRING);
        step = resolve_enum<Step>(_step.get<ScopedEnumValue>());
    }
};

struct CleanRule : Rule {
    std::vector<std::string> targets;

    CleanRule(std::string _name, Value _targets) {
        name = std::move(_name);
        Value::assert_types({{_targets, ValueType::LIST}});
        targets = ValueUtils::vectorise<std::string>(_targets.get<ValueList>(), ValueType::STRING);
    }
};

#endif