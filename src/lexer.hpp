#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <unordered_map>
#include <vector>

enum class LexemeType {
    IDENTIFIER,
    SINGLE_RULE_IDENTIFIER,
    ADD,
    EQUALS,
    LINE_START,
    BLOCK_START,
    BLOCK_END,
    LIST_START,
    LIST_END,
    MACRO_FN_START,
    MACRO_FN_END,
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
    size_t line_no;
    size_t col_no;
    size_t file_idx;
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
    constexpr static char MACRO_FN_START = '(';
    constexpr static char MACRO_FN_END = ')';
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
         {MACRO_FN_START, LexemeType::MACRO_FN_START},
         {MACRO_FN_END, LexemeType::MACRO_FN_END},
         {DELIMETER, LexemeType::DELIMETER},
         {EQUALS_CHAR, LexemeType::EQUALS},
         {ADD_CHAR, LexemeType::ADD}}};

    constexpr static std::string DEFAULT_SRC_FILE_NAME = "Buildfile";
    std::string src;
    // The absolute offset from the start of the stringified file
    size_t file_pos = 0;

    size_t line_no = 0;
    // Offset from the start of the current line
    size_t col_no = 0;

    std::vector<size_t> line_starts;

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

    void lex_rule_id(std::vector<Lexeme>& lexemes);

    void lex_identifier(std::vector<Lexeme>& lexemes);

    Lexeme make_lexeme(LexemeType type, std::string val = "") const;
};

#endif