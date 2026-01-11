#include "built_in_functions.hpp"

#include <algorithm>
#include <exception>
#include <ranges>
#include <stdexcept>

#include "value.hpp"

Value BuildInFuncRegistry::file_names(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::invalid_argument("Invalid argument count");
    }
    const Value& arg = args.at(0);
    if (arg.get_type() != ValueType::LIST) {
        throw std::invalid_argument("Argument is not a list");
    }

    ValueList stripped;

    const ValueList& files = arg.get<ValueList>();
    for (const auto& f : files) {
        if (f->get_type() != ValueType::STRING) {
            throw std::invalid_argument("Argument list contains a string");
        }

        std::string s = f->get<std::string>();
        size_t last_dot = s.find_last_of('.');
        if (last_dot != std::string::npos) {
            s.erase(last_dot);
        }

        stripped.push_back(std::make_unique<Value>(s));
    }
    return Value{std::move(stripped)};
}

Value BuildInFuncRegistry::call(const std::string& name, const std::vector<Value>& args) {
    if (!func_map.contains(name)) {
        throw std::invalid_argument("Cannot resolve function name '" + name + "'");
    }

    try {
        return func_map.at(name)(args);
    } catch (const std::exception& file_names) {
        throw std::invalid_argument("Error in function '" + name + "': " + file_names.what());
    }
}