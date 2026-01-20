#include "config.hpp"

#include <functional>

Config::Config(std::string _name, Value cfg_val) : name(_name) {
    cfg_val.assert_type(ValueType::Dictionary);
    Dictionary dict = cfg_val.get<Dictionary>();
    dict.assert_contains({{COMPILER_FIELD, ValueType::STRING}, {DEFAULT_FIELD, ValueType::STRING}});

    compiler = dict.get(COMPILER_FIELD).get<std::string>();
    default_rule = dict.get(DEFAULT_FIELD).get<std::string>();

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
}

const std::string& Config::get_compiler() const { return compiler; }

const std::vector<std::string>& Config::get_compilation_flags() const { return compilation_flags; }

const std::vector<std::string>& Config::get_link_flags() const { return link_flags; }

const std::string& Config::get_default_rule() const { return default_rule; }
