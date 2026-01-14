#include "lexer.hpp"
#include "parsing/parser.hpp"

int main(void) {
    std::vector<Lexeme> lexed = lex_file();
    Parser parser(lexed);
    std::vector<ParsedVariable> parsed = parser.parse();
    return 0;
}