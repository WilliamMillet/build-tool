#ifndef VALUE_REGISTRY_H
#define VALUE_REGISTRY_H

// #include <functional>
// #include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "lexer.hpp"
#include "value.hpp"

struct UnevaluatedVar {
    std::string name;
    std::vector<Lexeme> lexemes;
};

enum class BinaryOpType { ADD };

struct Expr {
    virtual ~Expr() = 0;
};

struct BinaryOpExpr : Expr {
    BinaryOpType type;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    BinaryOpExpr(BinaryOpType _type, std::unique_ptr<Expr> _left, std::unique_ptr<Expr> _right)
        : type(_type), left(std::move(_left)), right(std::move(_right)) {};
};

struct StringExpr : Expr {
    std::string val;

    StringExpr(std::string s) : val(std::move(s)) {};
};

struct VarRefExpr : Expr {
    std::string var_name;

    VarRefExpr(std::string s) : var_name(std::move(s)) {};
};

struct FnExpr : Expr {
    std::string func_name;
    std::vector<std::unique_ptr<Expr>> args;

    FnExpr(std::string fn_name) : func_name(std::move(fn_name)) {};
};

class Parser {
   public:
    Parser(std::vector<Lexeme> lexemes);

   private:
    std::unordered_map<std::string, Value> variables;
    std::vector<Lexeme> lexemes;
    size_t parse_pos = 0;

    inline const static std::vector<LexemeType> VALID_IDENTIFIER_SUCCESSORS = {
        LexemeType::EQUALS, LexemeType::BLOCK_START};

    inline const static std::vector<LexemeType> VARIABLE_STARTS = {
        LexemeType::IDENTIFIER, LexemeType::LIST_START, LexemeType::MACRO_FN_START,
        LexemeType::STRING};

    inline const static std::vector<LexemeType> INFIX_OPERATORS = {LexemeType::ADD};

    /** Get the lexeme at the current position */
    Lexeme peek() const;

    Lexeme peek_next() const;

    /** Advance the position in the lexemes and return what was 'consumed' */
    Lexeme consume();

    /** Consume as usual if the lexeme at the current position is lex. Otherwise throw */
    Lexeme consume(LexemeType lex);

    /** True iff the position is after all lexemes */
    bool at_end() const;

    /** Change the lexeme source the parser reads from and initialise the parse pos to 0 */
    void change_lexeme_source(std::vector<Lexeme>&& new_source);

    /** Throw an error with the line specified at the current lexeme line number */
    void throw_pinpointed_err [[noreturn]] (std::string msg) const;

    /** Assert that the type at the current position in the lexeme stream is in a given list */
    void expect_type(std::vector<LexemeType> type_pool) const;

    /** Return true iff the next tokens type is in a given list */
    bool match_type(std::vector<LexemeType> type_pool) const;

    /** Parse what is potentially an assignment, knowing it starts with an identifier */
    std::unique_ptr<UnevaluatedVar> parse_potential_assignment();

    /**
     * Consume a lexeme expression that is being assigned to a val and return it in unevaluated
     * form
     * @param identifier The identifier the lexeme expression is being assigned to
     */
    UnevaluatedVar consume_expr_as_unevaluated_var(std::string assigning_to);

    /** Parse one or more terms separated by additive operators */
    std::unique_ptr<Expr> parse_expr();

    /** Parse one or more terms separated by multiplicative operators*/
    std::unique_ptr<Expr> parse_term();

    /** Parse the arguments of a function from the opening to closing parenthesis */
    std::unique_ptr<FnExpr> parse_fn_args(std::string&& fn_name);
};

#endif