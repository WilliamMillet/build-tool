#include "lexer.hpp"
#include "parsing/parser.hpp"

int main(void) {
    Lexer lexer;
    std::vector<Lexeme> lexed = lexer.lex();
    Parser parser(lexed);
    std::vector<ParsedVariable> parsed = parser.parse();
    return 0;
}