#include "rules.hpp"

SingleRule::SingleRule(std::string _name, Value obj) {
    name = std::move(_name);
    obj.assert_type(ValueType::Dictionary);
    Dictionary dict = obj.get<Dictionary>();
    dict.assert_contains(
        {{RuleFields::NAME, ValueType::STRING}, {RuleFields::STEP, ValueType::ENUM}});

    deps = ValueUtils::vectorise<std::string>(dict.get(RuleFields::NAME).get<ValueList>());
    step = resolve_enum<Step>(dict.get(RuleFields::STEP).get<ScopedEnumValue>());
}

MultiRule::MultiRule(std::string _name, Value obj) {
    name = std::move(_name);
    obj.assert_type(ValueType::Dictionary);
    Dictionary dict = obj.get<Dictionary>();
    dict.assert_contains({{"deps", ValueType::LIST},
                          {RuleFields::OUTPUT, ValueType::LIST},
                          {RuleFields::STEP, ValueType::ENUM}});

    deps = ValueUtils::vectorise<std::string>(dict.get(RuleFields::NAME).get<ValueList>());
    output = ValueUtils::vectorise<std::string>(dict.get(RuleFields::OUTPUT).get<ValueList>());
    step = resolve_enum<Step>(dict.get(RuleFields::STEP).get<ScopedEnumValue>());
}

CleanRule::CleanRule(std::string _name, Value obj) {
    name = std::move(_name);
    obj.assert_type(ValueType::Dictionary);
    Dictionary dict = obj.get<Dictionary>();
    dict.assert_contains({{RuleFields::TARGETS, ValueType::LIST}});
    deps = ValueUtils::vectorise<std::string>(dict.get(RuleFields::NAME).get<ValueList>());
}