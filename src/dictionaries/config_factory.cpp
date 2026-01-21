#include "config_factory.hpp"

#include "config.hpp"

Config ConfigFactory::make_config(std::string id, Value cfg_val) const {
    cfg_val.assert_type(ValueType::Dictionary);
    Dictionary dict = cfg_val.get<Dictionary>();
    dict.assert_contains({{COMPILER_FIELD, ValueType::STRING}, {DEFAULT_FIELD, ValueType::STRING}});

    auto compiler = dict.get(COMPILER_FIELD).get<std::string>();
    auto default_rule = dict.get(DEFAULT_FIELD).get<std::string>();

    std::vector<std::string> compilation_flags;
    std::vector<std::string> link_flags;

    std::vector<std::pair<std::string, std::vector<std::string>*>> flag_pair = {
        {COMPILATION_FLAGS_FIELD, &compilation_flags}, {LINK_FLAGS_FIELD, &link_flags}};
    for (auto& [field_name, out] : flag_pair) {
        if (dict.contains(field_name)) {
            dict.assert_contains({{field_name, ValueType::LIST}});
            ValueList flag_list = dict.get(field_name).get<ValueList>();
            *out = ValueUtils::vectorise<std::string>(dict.get(field_name).get<ValueList>());
        }
    }

    if (dict.contains(DEFAULT_FIELD)) {
        dict.assert_contains({{DEFAULT_FIELD, ValueType::STRING}});
        default_rule = dict.get(DEFAULT_FIELD).get<std::string>();
    }

    return {std::move(id), std::move(compiler), std::move(compilation_flags), std::move(link_flags),
            std::move(default_rule)};
}
