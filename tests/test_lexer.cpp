#include <filesystem>
#include <vector>

#include "../catch.hpp"
#include "../src/lexer.hpp"
#include "src/errors/error.hpp"

inline constexpr std::string TEST_DATA_DIR = "../tests/data";

std::filesystem::path get_path(std::string file) {
    return std::filesystem::path(TEST_DATA_DIR + "/" + file);
}

TEST_CASE("Empty file has only a single EOF lexeme", "[lexer]") {
    Lexer lexer(get_path("Empty.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();
    REQUIRE(lexemes.size() == 1);

    Lexeme eof = lexemes[0];
    REQUIRE(eof.type == LexemeType::END_OF_FILE);
    REQUIRE(eof.loc == Location{1, 1, 0});
}

std::vector<Lexeme> simple_var_exp = {
    // Line 1: 'word = "Apple" # Should be ignored'
    {LexemeType::IDENTIFIER, "word", {1, 1, 0}},
    {LexemeType::EQUALS, "=", {1, 6, 5}},
    {LexemeType::STRING, "Apple", {1, 8, 7}},
    {LexemeType::NEWLINE, "\n", {1, 35, 34}},

    // Line 2: 'array = ["Dog", "Cat"]'
    {LexemeType::IDENTIFIER, "array", {2, 1, 35}},
    {LexemeType::EQUALS, "=", {2, 7, 41}},
    {LexemeType::LIST_START, "[", {2, 9, 43}},
    {LexemeType::STRING, "Dog", {2, 10, 44}},
    {LexemeType::DELIMETER, ",", {2, 15, 49}},
    {LexemeType::STRING, "Cat", {2, 17, 51}},
    {LexemeType::LIST_END, "]", {2, 22, 56}},
    {LexemeType::NEWLINE, "\n", {2, 23, 57}},

    // Line 3: 'enum = Step::LINK'
    {LexemeType::IDENTIFIER, "enum", {3, 1, 58}},
    {LexemeType::EQUALS, "=", {3, 6, 63}},
    {LexemeType::IDENTIFIER, "Step", {3, 8, 65}},
    {LexemeType::SCOPE_RESOLVER, "::", {3, 12, 69}},
    {LexemeType::IDENTIFIER, "LINK", {3, 14, 71}},
    {LexemeType::NEWLINE, "\n", {3, 18, 75}},
    // Line 4: 'EOF'
    {LexemeType::END_OF_FILE, "", {4, 1, 76}}};

TEST_CASE("Test simple variables", "[lexer]") {
    Lexer lexer(get_path("SimpleVariables.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    REQUIRE(lexemes.size() == simple_var_exp.size());

    for (size_t i = 0; i < simple_var_exp.size(); i++) {
        const Lexeme& exp = simple_var_exp.at(i);
        const Lexeme& got = lexemes.at(i);

        INFO("Comparing '" + got.value + "' with '" + exp.value + "'");
        INFO("Lexeme index: " + std::to_string(i));

        REQUIRE(got.type == exp.type);
        REQUIRE(got.loc == exp.loc);
        REQUIRE(got.value == exp.value);
    }
}

std::vector<Lexeme> complex_cfg_exp = {
    // Line 1: <Config> cfg {
    {LexemeType::DICT_QUALIFIER, "Config", {1, 1, 0}},
    {LexemeType::IDENTIFIER, "cfg", {1, 10, 9}},
    {LexemeType::BLOCK_START, "{", {1, 14, 13}},
    {LexemeType::NEWLINE, "\n", {1, 15, 14}},

    // Line 2: compiler = "clang++"
    {LexemeType::IDENTIFIER, "compiler", {2, 5, 19}},
    {LexemeType::EQUALS, "=", {2, 14, 28}},
    {LexemeType::STRING, "clang++", {2, 16, 30}},
    {LexemeType::NEWLINE, "\n", {2, 25, 39}},

    // Line 3: compilation_flags = ["-g", "-Wall"]
    {LexemeType::IDENTIFIER, "compilation_flags", {3, 5, 44}},
    {LexemeType::EQUALS, "=", {3, 23, 62}},
    {LexemeType::LIST_START, "[", {3, 25, 64}},
    {LexemeType::STRING, "-g", {3, 26, 65}},
    {LexemeType::DELIMETER, ",", {3, 30, 69}},
    {LexemeType::STRING, "-Wall", {3, 32, 71}},
    {LexemeType::LIST_END, "]", {3, 39, 78}},
    {LexemeType::NEWLINE, "\n", {3, 40, 79}},

    // Line 4: link_flags = []
    {LexemeType::IDENTIFIER, "link_flags", {4, 5, 84}},
    {LexemeType::EQUALS, "=", {4, 16, 95}},
    {LexemeType::LIST_START, "[", {4, 18, 97}},
    {LexemeType::LIST_END, "]", {4, 19, 98}},
    {LexemeType::NEWLINE, "\n", {4, 20, 99}},

    // Line 5: default = "app" # ...
    {LexemeType::IDENTIFIER, "default", {5, 5, 104}},
    {LexemeType::EQUALS, "=", {5, 13, 112}},
    {LexemeType::STRING, "app", {5, 15, 114}},
    {LexemeType::NEWLINE, "\n", {5, 20, 119}},

    // Line 6 & 7
    {LexemeType::BLOCK_END, "}", {6, 1, 121}},
    {LexemeType::NEWLINE, "\n", {6, 2, 122}},
    {LexemeType::NEWLINE, "\n", {7, 1, 123}},

    // Line 8: <MultiRule> compilation {
    {LexemeType::DICT_QUALIFIER, "MultiRule", {8, 1, 124}},
    {LexemeType::IDENTIFIER, "compilation", {8, 13, 136}},
    {LexemeType::BLOCK_START, "{", {8, 25, 148}},
    {LexemeType::NEWLINE, "\n", {8, 26, 149}},

    // Line 9: deps = cpp_files
    {LexemeType::IDENTIFIER, "deps", {9, 5, 154}},
    {LexemeType::EQUALS, "=", {9, 10, 159}},
    {LexemeType::IDENTIFIER, "cpp_files", {9, 12, 161}},
    {LexemeType::NEWLINE, "\n", {9, 21, 170}},

    // Line 10: output = cpp_names
    {LexemeType::IDENTIFIER, "output", {10, 5, 175}},
    {LexemeType::EQUALS, "=", {10, 12, 182}},
    {LexemeType::IDENTIFIER, "cpp_names", {10, 14, 184}},
    {LexemeType::NEWLINE, "\n", {10, 23, 193}},

    // Line 11: step = Step::Compile
    {LexemeType::IDENTIFIER, "step", {11, 5, 198}},
    {LexemeType::EQUALS, "=", {11, 10, 203}},
    {LexemeType::IDENTIFIER, "Step", {11, 12, 205}},
    {LexemeType::SCOPE_RESOLVER, "::", {11, 16, 209}},
    {LexemeType::IDENTIFIER, "Compile", {11, 18, 211}},
    {LexemeType::NEWLINE, "\n", {11, 25, 218}},

    // Line 12: }
    {LexemeType::BLOCK_END, "}", {12, 1, 220}},
    {LexemeType::END_OF_FILE, "", {12, 2, 221}}};

TEST_CASE("Test simple dictionaries", "[lexer]") {
    Lexer lexer(get_path("SimpleDicts.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    REQUIRE(lexemes.size() == complex_cfg_exp.size());

    for (size_t i = 0; i < complex_cfg_exp.size(); i++) {
        const Lexeme& exp = complex_cfg_exp.at(i);
        const Lexeme& got = lexemes.at(i);

        INFO("Comparing '" + got.value + "' with '" + exp.value + "'");
        INFO("Lexeme index: " + std::to_string(i));

        REQUIRE(got.type == exp.type);
        REQUIRE(got.loc == exp.loc);
        REQUIRE(got.value == exp.value);
    }
}
