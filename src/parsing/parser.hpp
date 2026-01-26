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
     * Parse the current lexeme source and return all the variables found. Function is single use
     * and likely won't work if called more then once as the lexeme source and parse_pos can change
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

    /** Advance the position in the lexemes and return what was 'consumed' */
    Lexeme consume();

    /** Consume as usual if the lexeme at the current position is lex. Otherwise throw */
    Lexeme consume(LexemeType lex);

    /** True iff the position is after all lexemes */
    bool at_end() const;

    /** Get the current location or EOF if all lexemes have been parsed */
    Location get_loc() const;

    /** Change the lexeme source the parser reads from and initialise the parse pos to 0 */
    void change_lexeme_source(std::vector<Lexeme>&& new_source);

    /** Assert that the type at the current position in the lexeme stream is in a given list */
    void expect_type(std::vector<LexemeType> type_pool) const;

    /** Return true iff the next tokens type is in a given list */
    bool match_type(std::vector<LexemeType> type_pool) const;

    /** Parse an assignment from the position after the dest identifier has been parsed */
    std::vector<Lexeme> consume_var_lexemes();

    /** Parse a config object assignment from the opening parenthesis */
    std::vector<Lexeme> consume_dict_lexemes();

    /** Parse one or more terms separated by additive operators */
    std::unique_ptr<Expr> parse_expr();

    /** Parse one or more terms separated by multiplicative operators*/
    std::unique_ptr<Expr> parse_term();

    /** Parse the arguments of a function from the opening to closing parenthesis */
    std::unique_ptr<FnExpr> parse_fn(std::string fn_name);

    /** Parse a list from the opening to closing parenthesis */
    std::unique_ptr<ListExpr> parse_list();

    /** Parse a Dictionary from the opening brace to and including the closing brace */
    std::unique_ptr<DictionaryExpr> parse_dictionary();

    /** Determine the category of a dictionary from it's qualifier id */
    VarCategory categorise_dictionary(const std::string& id);
};

#endif