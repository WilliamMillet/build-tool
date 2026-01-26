#include "func_registry.hpp"

#include <exception>

#include "../errors/error.hpp"
#include "../value.hpp"

Value FuncRegistry::call(const std::string& name, const std::vector<Value>& args) try {
    if (!func_map.contains(name)) {
        throw ValueError("Cannot resolve function name '" + name + "'");
    }

    return func_map.at(name)(args);
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Resolving function call");
}