#ifndef BUILT_IN_FUNCS_H
#define BUILT_IN_FUNCS_H

#include "../value.hpp"

namespace BuiltIn {
/** Strips all file extensions off a list of file names */
Value file_names(const std::vector<Value>& arg);
}  // namespace BuiltIn

#endif
