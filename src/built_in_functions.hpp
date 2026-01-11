#ifndef BUILT_IN_FUNCS_H
#define BUILT_IN_FUNCS_H

#include <functional>
#include <unordered_map>
#include <vector>

#include "value.hpp"

using BuiltInFunc = std::function<Value(const std::vector<Value>&)>;

class BuildInFuncRegistry {
   public:
    static Value call(const std::string& name, const std::vector<Value>& args);

   private:
    static Value file_names(const std::vector<Value>& arg);

    inline static const std::unordered_map<std::string, BuiltInFunc> func_map = {
        {"file_names", file_names}};
};

#endif