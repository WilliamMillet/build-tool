#include "funcs.hpp"

#include <filesystem>
#include <memory>
#include <unordered_set>
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
    if (args.size() != 2) {
        throw ValueError("Invalid argument count. 2 required: <path> <extensions>");
    }

    const Value& arg1 = args.at(0);
    if (arg1.get_type() != ValueType::STRING) {
        throw TypeError("Argument 1 is not a string");
    }
    const std::string path = arg1.get<std::string>();

    const Value& arg2 = args.at(1);
    if (arg2.get_type() != ValueType::LIST) {
        throw TypeError("Argument 1 is not a string");
    }
    const ValueList& ext_vlist = arg2.get<ValueList>();
    std::unordered_set<std::string> extensions;
    for (const Value& v : ext_vlist) {
        if (v.get_type() != ValueType::STRING) {
            throw TypeError("ValueList provided for argument 2 contains a non-string");
        }
        extensions.insert(v.get<std::string>());
    }

    std::vector<std::unique_ptr<Value>> files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const auto filename = entry.path().filename();
        if (filename.has_extension() && extensions.contains(filename.extension())) {
            files.push_back(std::make_unique<Value>(filename));
        }
    }

    return Value(ValueList(std::move(files)));
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Calling function 'files'");
}