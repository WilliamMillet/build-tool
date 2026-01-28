#ifndef BUILT_IN_ENUMS_H
#define BUILT_IN_ENUMS_H

#include "../errors/error.hpp"
#include "../value.hpp"

enum class Step { COMPILE, LINK };

/**
 * Given a ScopedEnumValue, get the actual enum itself
 * @tparam The specific enum type
 * @param val the value of the enum
 * @throws If the enum cannot be parsed (template not valid, scope not valid or name not valid)
 */
template <typename T>
T resolve_enum(ScopedEnumValue val) {
    const std::string enum_str = val.scope + "::" + val.name;

    throw ValueError("Failed to parse enum '" + enum_str + "'. Unknown type '" + val.scope + "'");
}

template <>
Step resolve_enum<Step>(ScopedEnumValue val);

#endif