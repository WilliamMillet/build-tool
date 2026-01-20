#include "../catch.hpp"
#include "src/lexer.hpp"
#include "src/parsing/expr.hpp"
#include "src/parsing/parser.hpp"

TEST_CASE("Test parser with empty file", "[parser]") {
    std::vector<Lexeme> lexemes = {{LexemeType::END_OF_FILE, "", Location{1, 1, 0}}};

    Parser parser(lexemes);
    std::vector<ParsedVariable> parsed = parser.parse();
    REQUIRE(parsed.size() == 0);
}

TEST_CASE("Test parser with simple variables", "[parser]") {
    // word = "Apple"
    // enum = Step::LINK
    // my_list = ["Dog", "Cat"]
    // EOF
    // There is no original file, so we set the location to an index in the vector for easier debug
    std::vector<Lexeme> lexemes = {{LexemeType::IDENTIFIER, "word", Location{0, 0, 0}},
                                   {LexemeType::EQUALS, "=", Location{0, 0, 1}},
                                   {LexemeType::STRING, "Apple", Location{0, 0, 2}},
                                   {LexemeType::NEWLINE, "\n", Location{0, 0, 3}},

                                   {LexemeType::IDENTIFIER, "enum", Location{0, 0, 4}},
                                   {LexemeType::EQUALS, "=", Location{0, 0, 5}},
                                   {LexemeType::IDENTIFIER, "Step", Location{0, 0, 6}},
                                   {LexemeType::SCOPE_RESOLVER, "::", Location{0, 0, 7}},
                                   {LexemeType::IDENTIFIER, "LINK", Location{0, 0, 8}},
                                   {LexemeType::NEWLINE, "\n", Location{0, 0, 9}},

                                   {LexemeType::IDENTIFIER, "my_list", Location{0, 0, 10}},
                                   {LexemeType::EQUALS, "=", Location{0, 0, 11}},
                                   {LexemeType::LIST_START, "[", Location{0, 0, 12}},
                                   {LexemeType::STRING, "Dog", Location{0, 0, 13}},
                                   {LexemeType::DELIMETER, ",", Location{0, 0, 14}},
                                   {LexemeType::STRING, "Cat", Location{0, 0, 15}},
                                   {LexemeType::LIST_END, "]", Location{0, 0, 16}},
                                   {LexemeType::NEWLINE, "\n", Location{0, 0, 17}},

                                   {LexemeType::END_OF_FILE, "", Location{0, 0, 18}}};

    Parser parser(lexemes);
    std::vector<ParsedVariable> parsed = parser.parse();

    REQUIRE(parsed.size() == 3);

    ParsedVariable& str_var = parsed.at(0);
    REQUIRE(str_var.identifier == "word");
    REQUIRE(str_var.category == VarCategory::REGULAR);
    StringExpr* str_expr = dynamic_cast<StringExpr*>(str_var.expr.get());
    REQUIRE(str_expr != nullptr);
    REQUIRE(str_expr->get_children().size() == 0);
    REQUIRE(str_expr->val == "Apple");
}
