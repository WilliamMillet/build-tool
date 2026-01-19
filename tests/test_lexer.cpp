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
    // Line 1: <Config> cfg { (14 chars + \n = 15 total)
    {LexemeType::DICT_QUALIFIER, "Config", {1, 1, 0}},
    {LexemeType::IDENTIFIER, "cfg", {1, 10, 9}},
    {LexemeType::BLOCK_START, "{", {1, 14, 13}},
    {LexemeType::NEWLINE, "\n", {1, 15, 14}},

    // Line 2:     compiler = "clang++" (24 chars + \n = 25 total)
    {LexemeType::IDENTIFIER, "compiler", {2, 5, 19}},
    {LexemeType::EQUALS, "=", {2, 14, 28}},
    {LexemeType::STRING, "clang++", {2, 16, 30}},
    {LexemeType::NEWLINE, "\n", {2, 25, 39}},

    // Line 3:     compilation_flags = ["-g", "-Wall"] (39 chars + \n = 40 total)
    {LexemeType::IDENTIFIER, "compilation_flags", {3, 5, 44}},
    {LexemeType::EQUALS, "=", {3, 23, 62}},
    {LexemeType::LIST_START, "[", {3, 25, 64}},
    {LexemeType::STRING, "-g", {3, 26, 65}},
    {LexemeType::DELIMETER, ",", {3, 30, 69}},
    {LexemeType::STRING, "-Wall", {3, 32, 71}},
    {LexemeType::LIST_END, "]", {3, 39, 78}},
    {LexemeType::NEWLINE, "\n", {3, 40, 79}},

    // Line 4:     link_flags = [] (19 chars + \n = 20 total)
    {LexemeType::IDENTIFIER, "link_flags", {4, 5, 84}},
    {LexemeType::EQUALS, "=", {4, 16, 95}},
    {LexemeType::LIST_START, "[", {4, 18, 97}},
    {LexemeType::LIST_END, "]", {4, 19, 98}},
    {LexemeType::NEWLINE, "\n", {4, 20, 99}},

    // Line 5:     default = "app" # This is the equivalent of 'all' from make
    {LexemeType::IDENTIFIER, "default", {5, 5, 104}},
    {LexemeType::EQUALS, "=", {5, 13, 112}},
    {LexemeType::STRING, "app", {5, 15, 114}},
    {LexemeType::NEWLINE, "\n", {5, 64, 163}},  // 63 chars + \n

    // Line 6 & 7: } \n \n
    {LexemeType::BLOCK_END, "}", {6, 1, 164}},
    {LexemeType::NEWLINE, "\n", {6, 2, 165}},
    {LexemeType::NEWLINE, "\n", {7, 1, 166}},

    // Line 8: <MultiRule> compilation { (25 chars + \n = 26 total)
    {LexemeType::DICT_QUALIFIER, "MultiRule", {8, 1, 167}},
    {LexemeType::IDENTIFIER, "compilation", {8, 13, 179}},
    {LexemeType::BLOCK_START, "{", {8, 25, 191}},
    {LexemeType::NEWLINE, "\n", {8, 26, 192}},

    // Line 9:     deps = cpp_files (20 chars + \n = 21 total)
    {LexemeType::IDENTIFIER, "deps", {9, 5, 197}},
    {LexemeType::EQUALS, "=", {9, 10, 202}},
    {LexemeType::IDENTIFIER, "cpp_files", {9, 12, 204}},
    {LexemeType::NEWLINE, "\n", {9, 21, 213}},

    // Line 10:     output = cpp_names (22 chars + \n = 23 total)
    {LexemeType::IDENTIFIER, "output", {10, 5, 218}},
    {LexemeType::EQUALS, "=", {10, 12, 225}},
    {LexemeType::IDENTIFIER, "cpp_names", {10, 14, 227}},
    {LexemeType::NEWLINE, "\n", {10, 23, 236}},

    // Line 11:     step = Step::Compile (24 chars + \n = 25 total)
    {LexemeType::IDENTIFIER, "step", {11, 5, 241}},
    {LexemeType::EQUALS, "=", {11, 10, 246}},
    {LexemeType::IDENTIFIER, "Step", {11, 12, 248}},
    {LexemeType::SCOPE_RESOLVER, "::", {11, 16, 252}},
    {LexemeType::IDENTIFIER, "Compile", {11, 18, 254}},
    {LexemeType::NEWLINE, "\n", {11, 25, 261}},

    // Line 12: }
    {LexemeType::BLOCK_END, "}", {12, 1, 262}},
    {LexemeType::END_OF_FILE, "", {12, 2, 263}}};

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
