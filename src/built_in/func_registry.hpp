#ifndef FUNC_REGISTRY_H
#define FUNC_REGISTRY_H

#include <functional>
#include <unordered_map>
#include <vector>

#include "../value.hpp"
#include "funcs.hpp"

using BuiltInFunc = std::function<Value(const std::vector<Value>&)>;
using FnMap = std::unordered_map<std::string, BuiltInFunc>;

Value file_names(const std::vector<Value>& arg);

class FuncRegistry {
   public:
    FuncRegistry(FnMap fn_map = DEFAULT_FN_MAP) : func_map(fn_map) {}

    Value call(const std::string& name, const std::vector<Value>& args);

   private:
    FnMap func_map;

    inline static const FnMap DEFAULT_FN_MAP = {{"file_names", BuiltIn::file_names}};
};

#endif