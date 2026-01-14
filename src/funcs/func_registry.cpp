#include "func_registry.hpp"

#include <exception>
#include <stdexcept>

#include "../value.hpp"

Value FuncRegistry::call(const std::string& name, const std::vector<Value>& args) {
    if (!func_map.contains(name)) {
        throw std::invalid_argument("Cannot resolve function name '" + name + "'");
    }

    try {
        return func_map.at(name)(args);
    } catch (const std::exception& file_names) {
        throw std::invalid_argument("Error in function '" + name + "': " + file_names.what());
    }
}