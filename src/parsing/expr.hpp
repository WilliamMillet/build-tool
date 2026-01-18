#ifndef EXPR_H
#define EXPR_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../built_in/func_registry.hpp"
#include "../value.hpp"

enum class BinaryOpType { ADD };

using VarMap = std::unordered_map<std::string, Value>;

class Expr {
   public:
    virtual ~Expr() = default;

    /** Singular interface for accessing node children (operands, arguments, etc) */
    virtual std::vector<Expr*> get_children() const = 0;

    /** Evaluate the expression tree as a value */
    virtual Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const = 0;
};

class BinaryOpExpr : public Expr {
   public:
    BinaryOpType type;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;

    BinaryOpExpr(BinaryOpType _type, std::unique_ptr<Expr> _left, std::unique_ptr<Expr> _right)
        : type(_type), left(std::move(_left)), right(std::move(_right)) {};
};

class StringExpr : public Expr {
   public:
    std::string val;

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;

    StringExpr(std::string s) : val(std::move(s)) {};
};

class EnumExpr : public Expr {
   public:
    std::string scope;
    std::string name;

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;
    EnumExpr(std::string _scope, std::string _name)
        : scope(std::move(_scope)), name(std::move(_name)) {};
};

class VarRefExpr : public Expr {
   public:
    std::string identifier;

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;

    VarRefExpr(std::string s) : identifier(std::move(s)) {};
};

class FnExpr : public Expr {
   public:
    std::string func_name;
    std::vector<std::unique_ptr<Expr>> args;

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;

    FnExpr(std::string fn_name) : func_name(std::move(fn_name)) {};
};

class DictionaryExpr : public Expr {
   public:
    std::unordered_map<std::string, std::unique_ptr<Expr>> fields_map;

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;
};

#endif
