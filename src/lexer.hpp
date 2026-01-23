#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "errors/error.hpp"

enum class LexemeType {
    IDENTIFIER,
    DICT_QUALIFIER,
    ADD,
    EQUALS,
    LINE_START,
    BLOCK_START,
    BLOCK_END,
    LIST_START,
    LIST_END,
    FN_START,
    FN_END,
    STRING,
    DELIMETER,
    SCOPE_RESOLVER,
    COMMENT,
    NEWLINE,
    END_OF_FILE,
};

struct Lexeme {
    LexemeType type;
    std::string value;
    Location loc;
};

class Lexer {
   public:
    Lexer(const std::string input = DEFAULT_SRC_FILE_NAME);

    /** Convert a file to lexemes */
    std::vector<Lexeme> lex();

   private:
    constexpr static char BLOCK_START = '{';
    constexpr static char BLOCK_END = '}';
    constexpr static char LIST_START = '[';
    constexpr static char LIST_END = ']';
    constexpr static char FN_START = '(';
    constexpr static char FN_END = ')';
    constexpr static char SINGLE_RULE_NAME_START = '<';
    constexpr static char SINGLE_RULE_NAME_END = '>';
    constexpr static char STRING_QUOTE = '"';
    constexpr static char SCOPE_RESOLVER = ':';
    constexpr static char DELIMETER = ',';
    constexpr static char EQUALS_CHAR = '=';
    constexpr static char ADD_CHAR = '+';
    constexpr static char COMMENT = '#';
    constexpr static char NEWLINE = '\n';

    // Characters that can only be part of a one sized token and map directly to a one sized lexeme
    inline const static std::unordered_map<char, LexemeType> DIRECT_MAPPINGS = {
        {{BLOCK_START, LexemeType::BLOCK_START},
         {BLOCK_END, LexemeType::BLOCK_END},
         {LIST_START, LexemeType::LIST_START},
         {LIST_END, LexemeType::LIST_END},
         {FN_START, LexemeType::FN_START},
         {FN_END, LexemeType::FN_END},
         {DELIMETER, LexemeType::DELIMETER},
         {EQUALS_CHAR, LexemeType::EQUALS},
         {ADD_CHAR, LexemeType::ADD}}};

    constexpr static std::string DEFAULT_SRC_FILE_NAME = "Buildfile.bf";
    std::string src;
    Location loc{.line_no = 1, .col_no = 1, .file_idx = 0};

    /** Returns the next character in the src */
    char peek() const;

    /** Return the next character in the src and advance the position */
    char consume();

    /** Consume if the char to consume is exp, otherwise throw an exception */
    char consume(char exp);

    /** Advance the character until a newline is reached. The newline is not consumed */
    void consume_line();

    /** True iff there are no more characters to lex */
    bool at_end() const;

    /** Determine if a character can be a valid component of an identifier */
    static bool valid_identifier_char(char c);

    void lex_string(std::vector<Lexeme>& lexemes);

    void lex_rule_qualifier(std::vector<Lexeme>& lexemes);

    void lex_identifier(std::vector<Lexeme>& lexemes);

    Lexeme make_lexeme(LexemeType type, std::string val = "") const;
};

#endif