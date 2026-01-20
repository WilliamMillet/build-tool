#include "expr.hpp"

#include <memory>
#include <ranges>
#include <string>
#include <vector>

BinaryOpExpr::BinaryOpExpr(BinaryOpType _type, std::unique_ptr<Expr> _left,
                           std::unique_ptr<Expr> _right)
    : type(_type), left(std::move(_left)), right(std::move(_right)) {};

std::vector<Expr*> BinaryOpExpr::get_children() const {
    std::vector<Expr*> res;

    if (left) res.push_back(left.get());
    if (right) res.push_back(right.get());

    return res;
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

StringExpr::StringExpr(std::string s) : val(std::move(s)) {};

std::vector<Expr*> StringExpr::get_children() const { return {}; }

Value StringExpr::evaluate(VarMap&, FuncRegistry&) const {
    std::string val_dup = val;
    return Value{std::move(val_dup)};
}

EnumExpr::EnumExpr(std::string _scope, std::string _name)
    : scope(std::move(_scope)), name(std::move(_name)) {};

std::vector<Expr*> EnumExpr::get_children() const { return {}; }

Value EnumExpr::evaluate(VarMap&, FuncRegistry&) const {
    return Value(ScopedEnumValue(scope, name));
};

VarRefExpr::VarRefExpr(std::string s) : identifier(std::move(s)) {}

std::vector<Expr*> VarRefExpr::get_children() const { return {}; }

Value VarRefExpr::evaluate(VarMap& var_map, FuncRegistry&) const {
    auto var_val = var_map.find(identifier);
    if (var_val == var_map.end()) {
        throw std::invalid_argument("Could not resolve variable '" + identifier + "'");
    }
    return var_val->second;
}

FnExpr::FnExpr(std::string fn_name) : func_name(std::move(fn_name)) {};

std::vector<Expr*> FnExpr::get_children() const {
    std::vector<Expr*> res;

    for (const std::unique_ptr<Expr>& child : args) {
        res.push_back(child.get());
    }

    return res;
}

Value FnExpr::evaluate(VarMap& var_map, FuncRegistry& fn_reg) const {
    std::vector<Value> arg_vals;
    for (const std::unique_ptr<Expr>& expr : args) {
        arg_vals.push_back(expr->evaluate(var_map, fn_reg));
    }

    return fn_reg.call(func_name, std::move(arg_vals));
}

ListExpr::ListExpr() {};

ListExpr::ListExpr(std::vector<std::unique_ptr<Expr>> elems) : elements(std::move(elems)) {}

std::vector<Expr*> ListExpr::get_children() const {
    std::vector<Expr*> res;

    for (const std::unique_ptr<Expr>& child : elements) {
        res.push_back(child.get());
    }

    return res;
}

Value ListExpr::evaluate(VarMap& var_map, FuncRegistry& fn_reg) const {
    std::vector<std::unique_ptr<Value>> elm_vals;
    for (const std::unique_ptr<Expr>& expr_elm : elements) {
        elm_vals.push_back(std::make_unique<Value>(expr_elm->evaluate(var_map, fn_reg)));
    }

    return Value(ValueList(std::move(elm_vals)));
}

std::vector<Expr*> DictionaryExpr::get_children() const {
    std::vector<Expr*> children;
    for (const std::unique_ptr<Expr>& child : std::views::values(fields_map)) {
        children.push_back(child.get());
    }
    return children;
}

Value DictionaryExpr::evaluate(VarMap& var_map, FuncRegistry& fn_reg) const {
    Dictionary Dictionary;

    for (const auto& [id, expr] : fields_map) {
        Dictionary.insert(id, expr->evaluate(var_map, fn_reg));
    }

    return Value{std::move(Dictionary)};
}
