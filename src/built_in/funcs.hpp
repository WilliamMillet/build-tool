#ifndef BUILT_IN_FUNCS_H
#define BUILT_IN_FUNCS_H

#include "../value.hpp"

namespace BuiltIn {
/**
 * Strips all file extensions off a list of file names
 * @param args[0] The list of file names
 */
Value file_names(const std::vector<Value>& arg);

/**
 * Recursively extracts all files in a directory with the required extension
 * @param args[0] The directory path string
 * @param args[1] The list of valid extensions
 */
Value files(const std::vector<Value>& args);

}  // namespace BuiltIn

#endif
