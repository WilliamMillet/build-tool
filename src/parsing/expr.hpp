#ifndef EXPR_H
#define EXPR_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../built_in/func_registry.hpp"
#include "../value.hpp"

enum class BinaryOpType { ADD };

// TODO - Maybe refactor this with public private

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

    BinaryOpExpr(BinaryOpType _type, std::unique_ptr<Expr> _left, std::unique_ptr<Expr> _right);

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;
};

class StringExpr : public Expr {
   public:
    std::string val;

    StringExpr(std::string s);

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;
};

class EnumExpr : public Expr {
   public:
    std::string scope;
    std::string name;
    EnumExpr(std::string _scope, std::string _name);

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;
};

class VarRefExpr : public Expr {
   public:
    std::string identifier;

    VarRefExpr(std::string s);

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;
};

class FnExpr : public Expr {
   public:
    std::string func_name;
    std::vector<std::unique_ptr<Expr>> args;

    FnExpr(std::string fn_name);

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;
};

class ListExpr : public Expr {
   public:
    std::vector<std::unique_ptr<Expr>> elements;

    ListExpr();

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;
};

class DictionaryExpr : public Expr {
   public:
    std::unordered_map<std::string, std::unique_ptr<Expr>> fields_map;

    std::vector<Expr*> get_children() const override;

    Value evaluate(VarMap& var_map, FuncRegistry& fn_reg) const override;
};

#endif
