#include "funcs.hpp"

#include <memory>
#include <vector>

#include "../value.hpp"

Value BuiltIn::file_names(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::invalid_argument("Invalid argument count");
    }
    const Value& arg = args.at(0);
    if (arg.get_type() != ValueType::LIST) {
        throw std::invalid_argument("Argument is not a list");
    }

    std::vector<std::unique_ptr<Value>> stripped;

    const ValueList& files = arg.get<ValueList>();
    for (const auto& f : files) {
        if (f.get_type() != ValueType::STRING) {
            throw std::invalid_argument("Argument list contains a string");
        }

        std::string s = f.get<std::string>();
        size_t last_dot = s.find_last_of('.');
        if (last_dot != std::string::npos) {
            s.erase(last_dot);
        }

        stripped.push_back(std::make_unique<Value>(s));
    }
    return Value{ValueList(std::move(stripped))};
}