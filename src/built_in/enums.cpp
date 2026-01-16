
#include "enums.hpp"

static void throw_scope_err [[noreturn]] (ScopedEnumValue val) {
    const std::string enum_str = val.scope + "::" + val.name;
    throw std::invalid_argument("Failed to parse enum '" + enum_str + "'. Unknown type '" +
                                val.scope + "'");
}

static void throw_name_err [[noreturn]] (ScopedEnumValue val) {
    const std::string enum_str = val.scope + "::" + val.name;
    throw std::invalid_argument("Failed to parse enum '" + enum_str + ". No member '" + val.name +
                                "' found");
}

template <>
Step resolve_enum<Step>(ScopedEnumValue val) {
    if (val.scope != "Step") throw_scope_err(val);

    if (val.name == "COMPILE") return Step::COMPILE;
    if (val.name == "LINK") return Step::LINK;

    throw_name_err(val);
}
