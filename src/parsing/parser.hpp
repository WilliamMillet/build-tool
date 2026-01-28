#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <string>
#include <vector>

#include "../lexer.hpp"
#include "expr.hpp"

enum class VarCategory {
    REGULAR,
    SINGLE_RULE,
    MULTI_RULE,
    CONFIG,
    CLEAN,
};

struct VarLexemes {
    std::string identifier;
    std::vector<Lexeme> lexemes;
    VarCategory category;
    Location start_loc;
};

struct ParsedVariable {
    std::string identifier;
    std::unique_ptr<Expr> expr;
    VarCategory category;
    Location loc;
};

class Parser {
   public:
    Parser(std::vector<Lexeme> _lexemes);

    /**
     * @brief Parse the current lexeme source and return all the variables found. Function is single
     * use and likely won't work if called more then once as the lexeme source and parse_pos can
     * change
     */
    std::vector<ParsedVariable> parse();

   private:
    std::vector<Lexeme> lexemes;
    size_t parse_pos = 0;

    inline const static std::vector<LexemeType> VARIABLE_STARTS = {
        LexemeType::IDENTIFIER, LexemeType::LIST_START, LexemeType::FN_START, LexemeType::STRING,
        LexemeType::BLOCK_START};

    inline const static std::vector<LexemeType> INFIX_OPERATORS = {LexemeType::ADD};

    /** Get the lexeme at the current position */
    Lexeme peek() const;

    /**
     * @brief Advance the position in the lexemes
     *
     * @return Lexeme The consumed lexeme
     */
    Lexeme consume();

    /**
     * @brief Consume a specific value.
     *
     * @param lex The expected lexeme type
     * @return Lexeme The consumed lexeme
     * @throws if the lexeme is not of the expected type or there are no more lexemes
     */
    Lexeme consume(LexemeType lex);

    /** True if and only if the position is after all lexemes */
    bool at_end() const;

    /** Get the current location or EOF if all lexemes have been parsed */
    Location get_loc() const;

    /** Change the lexeme source the parser reads from and initialise the parse pos to 0 */
    void change_lexeme_source(std::vector<Lexeme>&& new_source);

    /**
     * @brief Assert that the type at the current position in the lexeme stream
     *
     * @param type_pool The valid types that could be next
     * @throws if end of lexemes have been reached or the current lexeme is not in the pool
     */
    void expect_type(std::vector<LexemeType> type_pool) const;

    /** Return true iff the next tokens type is in a given list */
    bool match_type(std::vector<LexemeType> type_pool) const;

    /**
     * @brief From the current lexeme position, aggregate all of the lexemes in the variable
     * at that position. The lexeme position is advanced to the end
     *
     * @return std::vector<Lexeme> All lexemes belonging to a variable expression
     */
    std::vector<Lexeme> consume_var_lexemes();

    /**
     * @brief From the current lexeme position, aggregate all of the lexemes in the dictionary
     * at that position. The lexeme position is advanced to the end.
     *
     * @return std::vector<Lexeme> All lexemes belonging to the dictionary
     * @throws If an unmatched parenthesis is found
     */
    std::vector<Lexeme> consume_dict_lexemes();

    /** AST building functions*/

    /**
     * @brief Parse one or more terms separated by additive operators
     *
     * @return std::unique_ptr<Expr> The parsed expression
     * @throws If an unexpected operand is found
     */
    std::unique_ptr<Expr> parse_expr();

    /**
     * @brief Parse one or more terms separated by multiplicative operators
     *
     * @return std::unique_ptr<Expr> The parsed term
     * @throws if the term found is invalid
     */
    std::unique_ptr<Expr> parse_term();

    /**
     * @brief Parse the arguments of a function from the opening to closing parenthesis
     *
     * @param fn_name The function identifier
     * @return std::unique_ptr<FnExpr> The parsed function
     * @exception If there are unclosed brackets
     */
    std::unique_ptr<FnExpr> parse_fn(std::string fn_name);

    /** Parse a list from the opening to closing parenthesis */
    std::unique_ptr<ListExpr> parse_list();

    /**
     * @brief Parse a Dictionary from the opening brace to and including the closing brace
     *
     * @return std::unique_ptr<DictionaryExpr>
     * @throws If the end of the dictionary is reached prematurely
     */
    std::unique_ptr<DictionaryExpr> parse_dictionary();

    /**
     * @brief Determine the category of a dictionary from it's qualifier id
     *
     * @param id The string id qualifier representation
     * @return VarCategory The category enum
     * @throws If the category is unknown
     */
    VarCategory categorise_dictionary(const std::string& id);
};

#endif