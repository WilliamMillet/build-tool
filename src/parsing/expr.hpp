#ifndef EXPR_H
#define EXPR_H

#include <memory>
#include <string>
#include <vector>

#include "../funcs/func_registry.hpp"
#include "../value.hpp"

enum class BinaryOpType { ADD };

using VarMap = std::unordered_map<std::string, Value>;

struct Expr {
    virtual ~Expr() = 0;

    /** Singular interface for accessing node children (operands, arguments, etc) */
    virtual std::vector<Expr*> get_children() const = 0;

    /** Evaluate the expression tree as a value */
    virtual Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const = 0;
};

struct BinaryOpExpr : Expr {
    BinaryOpType type;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;

    BinaryOpExpr(BinaryOpType _type, std::unique_ptr<Expr> _left, std::unique_ptr<Expr> _right)
        : type(_type), left(std::move(_left)), right(std::move(_right)) {};
};

struct StringExpr : Expr {
    std::string val;

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;

    StringExpr(std::string s) : val(std::move(s)) {};
};

struct EnumExpr : Expr {
    std::string scope;
    std::string name;

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;
    EnumExpr(std::string _scope, std::string _name)
        : scope(std::move(_scope)), name(std::move(_name)) {};
};

struct VarRefExpr : Expr {
    std::string identifier;

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;

    VarRefExpr(std::string s) : identifier(std::move(s)) {};
};

struct FnExpr : Expr {
    std::string func_name;
    std::vector<std::unique_ptr<Expr>> args;

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;

    FnExpr(std::string fn_name) : func_name(std::move(fn_name)) {};
};

#endif