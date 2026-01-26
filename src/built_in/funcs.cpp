#include "funcs.hpp"

#include <filesystem>
#include <memory>
#include <vector>

#include "../errors/error.hpp"
#include "../value.hpp"

Value BuiltIn::file_names(const std::vector<Value>& args) try {
    if (args.size() != 1) {
        throw ValueError("Invalid argument count. Only 1 argument permitted");
    }
    const Value& arg = args.at(0);
    if (arg.get_type() != ValueType::LIST) {
        throw TypeError("Argument is not a list");
    }

    std::vector<std::unique_ptr<Value>> stripped;

    const ValueList& files = arg.get<ValueList>();
    for (const auto& f : files) {
        if (f.get_type() != ValueType::STRING) {
            throw TypeError("Argument list contains a string");
        }

        std::string s = f.get<std::string>();
        size_t last_dot = s.find_first_of('.');
        if (last_dot != std::string::npos) {
            s.erase(last_dot);
        }

        stripped.push_back(std::make_unique<Value>(s));
    }
    return Value{ValueList(std::move(stripped))};
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Calling function 'file_names'");
}

Value BuiltIn::files(const std::vector<Value>& args) try {
    if (args.size() != 1) {
        throw ValueError("Invalid argument count. Only 1 argument permitted");
    }

    const Value& arg = args.at(0);
    if (arg.get_type() != ValueType::STRING) {
        throw TypeError("Argument is not a string");
    }

    std::string path = arg.get<std::string>();

    std::vector<std::unique_ptr<Value>> files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        auto filename = entry.path().filename();
        if (filename.has_extension() && filename.extension() == ".cpp") {
            files.push_back(std::make_unique<Value>(filename));
        }
    }

    return Value(ValueList(std::move(files)));
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Calling function 'files'");
}