#include "rule_factory.hpp"

#include <memory>

std::unique_ptr<Rule> RuleFactory::make_rule(std::string name, Value obj, Location loc,
                                             VarCategory cat) const try {
    switch (cat) {
        case VarCategory::CLEAN: {
            return make_clean_rule(std::move(name), std::move(obj), std::move(loc));
        }
        case VarCategory::SINGLE_RULE: {
            return make_single_rule(std::move(name), std::move(obj), std::move(loc));
        }
        case VarCategory::MULTI_RULE: {
            return make_multi_rule(std::move(name), std::move(obj), std::move(loc));
        }
        default: {
            const std::string enum_str = std::to_string(static_cast<int>(cat));
            throw ValueError("Invalid rule category (Enum ID: '" + enum_str + "')");
        }
    }
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Rule factory method for '" + name + "'", loc);
}

std::unique_ptr<CleanRule> RuleFactory::make_clean_rule(std::string name, Value obj,
                                                        Location loc) const try {
    obj.assert_type(ValueType::Dictionary);
    const Dictionary dict = obj.get<Dictionary>();
    dict.assert_contains({{RuleFields::TARGETS, ValueType::LIST}});
    const auto deps =
        ValueUtils::vectorise<std::string>(dict.get(RuleFields::TARGETS).get<ValueList>());
    return std::make_unique<CleanRule>(name, deps, loc);
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "CleanRule factory method for '<CleanRule> " + name + "'", loc);
}

std::unique_ptr<MultiRule> RuleFactory::make_multi_rule(std::string name, Value obj,
                                                        Location loc) const try {
    obj.assert_type(ValueType::Dictionary);
    const Dictionary dict = obj.get<Dictionary>();
    dict.assert_contains({{RuleFields::DEPS, ValueType::LIST},
                          {RuleFields::OUTPUT, ValueType::LIST},
                          {RuleFields::STEP, ValueType::ENUM}});

    const auto deps =
        ValueUtils::vectorise<std::string>(dict.get(RuleFields::DEPS).get<ValueList>());
    const auto out =
        ValueUtils::vectorise<std::string>(dict.get(RuleFields::OUTPUT).get<ValueList>());

    if (deps.size() != out.size()) {
        throw ValueError("Error in MultiRule '" + name + "'. 'deps' length (" +
                         std::to_string(deps.size()) + ") is not the same as 'output' length (" +
                         std::to_string(out.size()) + ").");
    }

    const auto step = resolve_enum<Step>(dict.get(RuleFields::STEP).get<ScopedEnumValue>());

    return std::make_unique<MultiRule>(name, deps, out, step, loc);
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "MultiRule factory method for '<MultiRule> " + name + "'", loc);
}

std::unique_ptr<SingleRule> RuleFactory::make_single_rule(std::string name, Value obj,
                                                          Location loc) const try {
    obj.assert_type(ValueType::Dictionary);
    const Dictionary dict = obj.get<Dictionary>();
    dict.assert_contains(
        {{RuleFields::DEPS, ValueType::LIST}, {RuleFields::STEP, ValueType::ENUM}});

    const auto deps =
        ValueUtils::vectorise<std::string>(dict.get(RuleFields::DEPS).get<ValueList>());
    const auto step = resolve_enum<Step>(dict.get(RuleFields::STEP).get<ScopedEnumValue>());

    return std::make_unique<SingleRule>(name, deps, step, loc);
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "SingleRule factory method for '<Rule> " + name + "'", loc);
}