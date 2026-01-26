#include "expr.hpp"

#include <memory>
#include <ranges>
#include <string>
#include <vector>

#include "../errors/error.hpp"

BinaryOpExpr::BinaryOpExpr(BinaryOpType _type, std::unique_ptr<Expr> _left,
                           std::unique_ptr<Expr> _right)
    : type(_type), left(std::move(_left)), right(std::move(_right)) {};

std::vector<Expr*> BinaryOpExpr::get_children() const {
    std::vector<Expr*> res;

    if (left != nullptr) res.push_back(left.get());
    if (right != nullptr) res.push_back(right.get());

    return res;
}

Value BinaryOpExpr::evaluate(const VarMap& var_map, const FuncRegistry& fn_reg) const try {
    Value val = left->evaluate(var_map, fn_reg);
    switch (type) {
        case BinaryOpType::ADD: {
            val += right->evaluate(var_map, fn_reg);
        }
    }

    return val;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Evaluating binary operation expression");
}

StringExpr::StringExpr(std::string s) : val(std::move(s)) {};

std::vector<Expr*> StringExpr::get_children() const { return {}; }

Value StringExpr::evaluate(const VarMap&, const FuncRegistry&) const try {
    std::string val_dup = val;
    return Value{std::move(val_dup)};
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Evaluating string expression");
}

EnumExpr::EnumExpr(std::string _scope, std::string _name)
    : scope(std::move(_scope)), name(std::move(_name)) {};

std::vector<Expr*> EnumExpr::get_children() const { return {}; }

Value EnumExpr::evaluate(const VarMap&, const FuncRegistry&) const try {
    return Value(ScopedEnumValue(scope, name));
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Evaluating enum expression");
}

const std::string& EnumExpr::get_scope() const { return scope; };

const std::string& EnumExpr::get_name() const { return name; }

VarRefExpr::VarRefExpr(std::string s) : identifier(std::move(s)) {}

std::vector<Expr*> VarRefExpr::get_children() const { return {}; }

Value VarRefExpr::evaluate(const VarMap& var_map, const FuncRegistry&) const try {
    auto var_val = var_map.find(identifier);
    if (var_val == var_map.end()) {
        throw ValueError("Could not resolve variable '" + identifier + "'");
    }
    return var_val->second;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Evaluating variable reference expression");
}

const std::string& VarRefExpr::get_id() const { return identifier; }

FnExpr::FnExpr(std::string fn_name) : func_name(std::move(fn_name)) {};

std::vector<Expr*> FnExpr::get_children() const {
    std::vector<Expr*> res;

    for (const std::unique_ptr<Expr>& child : args) {
        res.push_back(child.get());
    }

    return res;
}

Value FnExpr::evaluate(const VarMap& var_map, const FuncRegistry& fn_reg) const try {
    std::vector<Value> arg_vals;
    for (const std::unique_ptr<Expr>& expr : args) {
        arg_vals.push_back(expr->evaluate(var_map, fn_reg));
    }

    return fn_reg.call(func_name, std::move(arg_vals));
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Evaluating function expression");
}

void FnExpr::add_arg(std::unique_ptr<Expr> arg) { args.push_back(std::move(arg)); }

ListExpr::ListExpr() {};

ListExpr::ListExpr(std::vector<std::unique_ptr<Expr>> elems) : elements(std::move(elems)) {}

std::vector<Expr*> ListExpr::get_children() const try {
    std::vector<Expr*> res;

    for (const std::unique_ptr<Expr>& child : elements) {
        res.push_back(child.get());
    }

    return res;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Evaluating list expression");
}

Value ListExpr::evaluate(const VarMap& var_map, const FuncRegistry& fn_reg) const {
    std::vector<std::unique_ptr<Value>> elm_vals;
    for (const std::unique_ptr<Expr>& expr_elm : elements) {
        elm_vals.push_back(std::make_unique<Value>(expr_elm->evaluate(var_map, fn_reg)));
    }

    return Value(ValueList(std::move(elm_vals)));
}

void ListExpr::append(std::unique_ptr<Expr> expr) { elements.push_back(std::move(expr)); }

const std::vector<std::unique_ptr<Expr>>& ListExpr::get_elements() const { return elements; };

std::vector<Expr*> DictionaryExpr::get_children() const {
    std::vector<Expr*> children;
    for (const std::unique_ptr<Expr>& child : std::views::values(fields_map)) {
        children.push_back(child.get());
    }
    return children;
}

Value DictionaryExpr::evaluate(const VarMap& var_map, const FuncRegistry& fn_reg) const try {
    Dictionary Dictionary;

    for (const auto& [id, expr] : fields_map) {
        Dictionary.insert(id, expr->evaluate(var_map, fn_reg));
    }

    return Value{std::move(Dictionary)};
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Evaluating dictionary expression");
}

void DictionaryExpr::insert_entry(std::string key, std::unique_ptr<Expr> val) {
    fields_map[key] = std::move(val);
}

const std::unordered_map<std::string, std::unique_ptr<Expr>>& DictionaryExpr::get_fields_map()
    const {
    return fields_map;
}