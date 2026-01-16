#include "expr.hpp"

#include <memory>
#include <ranges>
#include <string>
#include <vector>

std::vector<Expr*> BinaryOpExpr::get_children() const {
    std::vector<Expr*> res;

    if (left) res.push_back(left.get());
    if (right) res.push_back(right.get());

    return res;
}

std::vector<Expr*> StringExpr::get_children() const { return {}; }

std::vector<Expr*> EnumExpr::get_children() const { return {}; }

std::vector<Expr*> VarRefExpr::get_children() const { return {}; }

std::vector<Expr*> FnExpr::get_children() const {
    std::vector<Expr*> res;

    for (const std::unique_ptr<Expr>& child : args) {
        res.push_back(child.get());
    }

    return res;
}

std::vector<Expr*> DictionaryExpr::get_children() const {
    std::vector<Expr*> children;
    for (const std::unique_ptr<Expr>& child : std::views::values(fields_map)) {
        children.push_back(child.get());
    }
    return children;
}

Value BinaryOpExpr::evaluate(VarMap& var_map, FuncRegistry& fn_reg) const {
    Value val = left->evaluate(var_map, fn_reg);
    switch (type) {
        case BinaryOpType::ADD: {
            val += right->evaluate(var_map, fn_reg);
        }
    }
    return val;
}

Value StringExpr::evaluate(VarMap&, FuncRegistry&) const {
    std::string val_dup = val;
    return Value{std::move(val_dup)};
}

Value EnumExpr::evaluate(VarMap&, FuncRegistry&) const {
    return Value(ScopedEnumValue(scope, name));
};

Value VarRefExpr::evaluate(VarMap& var_map, FuncRegistry&) const {
    auto var_val = var_map.find(identifier);
    if (var_val == var_map.end()) {
        throw std::invalid_argument("Could not resolve variable '" + identifier + "'");
    }
    return var_val->second;
}

Value FnExpr::evaluate(VarMap& var_map, FuncRegistry& fn_reg) const {
    std::vector<Value> arg_vals;
    for (const std::unique_ptr<Expr>& expr : args) {
        arg_vals.push_back(expr->evaluate(var_map, fn_reg));
    }

    return fn_reg.call(func_name, std::move(arg_vals));
}

Value DictionaryExpr::evaluate(VarMap& var_map, FuncRegistry& fn_reg) const {
    Dictionary Dictionary;

    for (const auto& [id, expr] : fields_map) {
        Dictionary.insert(id, expr->evaluate(var_map, fn_reg));
    }

    return Value{std::move(Dictionary)};
}