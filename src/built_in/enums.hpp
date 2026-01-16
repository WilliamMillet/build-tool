#ifndef BUILT_IN_ENUMS_H
#define BUILT_IN_ENUMS_H

#include <stdexcept>

#include "../value.hpp"

enum class Step { COMPILE, LINK };

template <typename T>
T resolve_enum(ScopedEnumValue val) {
    const std::string enum_str = val.scope + "::" + val.name;

    throw std::invalid_argument("Failed to parse enum '" + enum_str + "'. Unknown type '" +
                                val.scope + "'");
}

template <>
Step resolve_enum<Step>(ScopedEnumValue val);

#endif