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

/** Registry of built in functions. All functions are pure */
class FuncRegistry {
   public:
    FuncRegistry(FnMap fn_map = DEFAULT_FN_MAP) : func_map(fn_map) {}

    /**
     * Call a function from the registry
     * @param name The name of the function
     * @param args The list of function arguments
     * @returns The result of the function
     * @throws If the function name cannot be resolved to a function. Errors are thrown in the
     * function if any arguments are invalid (among other reasons)
     */
    Value call(const std::string& name, const std::vector<Value>& args) const;

   private:
    FnMap func_map;

    inline static const FnMap DEFAULT_FN_MAP = {{"file_names", BuiltIn::file_names},
                                                {"files", BuiltIn::files}};
};

#endif