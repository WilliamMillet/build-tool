#include <memory>
#include <string>
#include <vector>

#include "value.hpp"

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
};

struct SingleRule : public Rule {
    std::string output;
};

struct MultiRule : public Rule {
    std::vector<std::string> ouput;
};

struct Clean {
    std::vector<std::string> targets;
};
