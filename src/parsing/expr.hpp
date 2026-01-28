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

    /**
     * @brief Get the children object. This acts as a singular interface for accessing node children
     *
     * @return std::vector<Expr*> The list of all subexpressions (operands, expressions in
     * containers, function arguments, etc)
     */
    virtual std::vector<Expr*> get_children() const = 0;

    /**
     * @brief Evaluate the expression tree as a value
     *
     * @param var_map The map of identifiers to variables
     * @param fn_reg The callable function registry
     * @return Value The evaluated value
     */
    virtual Value evaluate(const VarMap& var_map, const FuncRegistry& fn_reg) const = 0;
};

class BinaryOpExpr : public Expr {
   public:
    BinaryOpExpr(BinaryOpType _type, std::unique_ptr<Expr> _left, std::unique_ptr<Expr> _right);

    std::vector<Expr*> get_children() const override;

    Value evaluate(const VarMap& var_map, const FuncRegistry& fn_reg) const override;

   private:
    BinaryOpType type;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
};

class StringExpr : public Expr {
   public:
    std::string val;

    StringExpr(std::string s);

    std::vector<Expr*> get_children() const override;

    Value evaluate(const VarMap& var_map, const FuncRegistry& fn_reg) const override;
};

class EnumExpr : public Expr {
   public:
    EnumExpr(std::string _scope, std::string _name);

    std::vector<Expr*> get_children() const override;

    Value evaluate(const VarMap& var_map, const FuncRegistry& fn_reg) const override;

    const std::string& get_scope() const;
    const std::string& get_name() const;

   private:
    std::string scope;
    std::string name;
};

class VarRefExpr : public Expr {
   public:
    VarRefExpr(std::string s);

    std::vector<Expr*> get_children() const override;

    Value evaluate(const VarMap& var_map, const FuncRegistry& fn_reg) const override;

    const std::string& get_id() const;

   private:
    std::string identifier;
};

class FnExpr : public Expr {
   public:
    FnExpr(std::string fn_name);

    std::vector<Expr*> get_children() const override;

    Value evaluate(const VarMap& var_map, const FuncRegistry& fn_reg) const override;

    void add_arg(std::unique_ptr<Expr> arg);

   private:
    std::string func_name;
    std::vector<std::unique_ptr<Expr>> args;
};

class ListExpr : public Expr {
   public:
    ListExpr();
    ListExpr(std::vector<std::unique_ptr<Expr>> elems);

    std::vector<Expr*> get_children() const override;

    Value evaluate(const VarMap& var_map, const FuncRegistry& fn_reg) const override;

    void append(std::unique_ptr<Expr> expr);

    const std::vector<std::unique_ptr<Expr>>& get_elements() const;

   private:
    std::vector<std::unique_ptr<Expr>> elements;
};

class DictionaryExpr : public Expr {
   public:
    std::vector<Expr*> get_children() const override;

    Value evaluate(const VarMap& var_map, const FuncRegistry& fn_reg) const override;

    void insert_entry(std::string key, std::unique_ptr<Expr> val);

    const std::unordered_map<std::string, std::unique_ptr<Expr>>& get_fields_map() const;

   private:
    std::unordered_map<std::string, std::unique_ptr<Expr>> fields_map;
};

#endif
