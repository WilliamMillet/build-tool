#include "config.hpp"

Config::Config(Value cfg_obj) {
    cfg_obj.assert_type(ValueType::CFG_OBJ);
    Dictionary dict = cfg_obj.get<Dictionary>();
    dict.assert_contains(
        {{COMPILER_FIELD, ValueType::STRING}, {DEFAULT_RULE_FIELD, ValueType::STRING}});

    compiler = dict.get(COMPILER_FIELD).get<std::string>();
    default_rule = dict.get(DEFAULT_RULE_FIELD).get<std::string>();

    std::vector<std::pair<std::string, std::string>> flag_pair = {
        {COMPILATION_FLAGS_FIELD, compilation_flags}, {LINK_FLAGS_FIELD, link_flags}};
    for (auto& [field_name, out] : flag_pair) {
        if (dict.contains(field_name)) {
            dict.assert_contains({{field_name, ValueType::LIST}});
            ValueList flag_list = dict.get(field_name).get<ValueList>();
            out = join_list(flag_list);
        }
    }

    if (dict.contains(DEFAULT_RULE_FIELD)) {
        dict.assert_contains({{DEFAULT_RULE_FIELD, ValueType::LIST}});
        default_rule = dict.get(DEFAULT_RULE_FIELD).get<std::string>();
    }
}

const std::string& Config::get_compiler() const { return compiler; }

const std::string& Config::get_compilation_flags() const { return compilation_flags; }

const std::string& Config::get_link_flags() const { return link_flags; }

const std::string& Config::get_default_rule() const { return default_rule; }

std::string Config::join_list(ValueList list) const {
    std::string joined;
    for (Value& flag : list) {
        flag.assert_type(ValueType::STRING);
        if (!joined.empty()) {
            joined += " ";
        }
        joined += flag.get<std::string>();
    }

    return joined;
}
