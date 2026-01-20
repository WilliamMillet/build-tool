#include <filesystem>
#include <vector>

#include "../catch.hpp"
#include "../src/lexer.hpp"
#include "src/errors/error.hpp"

inline const std::string TEST_DATA_DIR = "../tests/data/files";

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

TEST_CASE("Lex simple variables", "[lexer]") {
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

TEST_CASE("Lex simple dictionaries", "[lexer]") {
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

std::vector<Lexeme> strange_data_exp = {
    // Line 1: # Syntax will not work but should be lexed correctly (52 chars + \n)
    {LexemeType::NEWLINE, "\n", {1, 53, 52}},

    // Line 2: cpp_files = files(src_dir) (26 chars + \n)
    {LexemeType::IDENTIFIER, "cpp_files", {2, 1, 53}},
    {LexemeType::EQUALS, "=", {2, 11, 63}},
    {LexemeType::IDENTIFIER, "files", {2, 13, 65}},
    {LexemeType::FN_START, "(", {2, 18, 70}},
    {LexemeType::IDENTIFIER, "src_dir", {2, 19, 71}},
    {LexemeType::FN_END, ")", {2, 26, 78}},
    {LexemeType::NEWLINE, "\n", {2, 27, 79}},

    // Line 3: <Clean> clean { (15 chars + \n)
    {LexemeType::DICT_QUALIFIER, "Clean", {3, 1, 80}},
    {LexemeType::IDENTIFIER, "clean", {3, 9, 88}},
    {LexemeType::BLOCK_START, "{", {3, 15, 94}},
    {LexemeType::NEWLINE, "\n", {3, 16, 95}},

    // Line 4:     remove = ["app", ["nested", [[[]]]], ] + file_names(cpp_files) (66 chars + \n)
    {LexemeType::IDENTIFIER, "remove", {4, 5, 100}},
    {LexemeType::EQUALS, "=", {4, 12, 107}},
    {LexemeType::LIST_START, "[", {4, 14, 109}},
    {LexemeType::STRING, "app", {4, 15, 110}},
    {LexemeType::DELIMETER, ",", {4, 20, 115}},
    {LexemeType::LIST_START, "[", {4, 22, 117}},
    {LexemeType::STRING, "nested", {4, 23, 118}},
    {LexemeType::DELIMETER, ",", {4, 31, 126}},
    {LexemeType::LIST_START, "[", {4, 33, 128}},
    {LexemeType::LIST_START, "[", {4, 34, 129}},
    {LexemeType::LIST_START, "[", {4, 35, 130}},
    {LexemeType::LIST_END, "]", {4, 36, 131}},
    {LexemeType::LIST_END, "]", {4, 37, 132}},
    {LexemeType::LIST_END, "]", {4, 38, 133}},
    {LexemeType::LIST_END, "]", {4, 39, 134}},
    {LexemeType::DELIMETER, ",", {4, 40, 135}},
    {LexemeType::LIST_END, "]", {4, 42, 137}},
    {LexemeType::ADD, "+", {4, 44, 139}},
    {LexemeType::IDENTIFIER, "file_names", {4, 46, 141}},
    {LexemeType::FN_START, "(", {4, 56, 151}},
    {LexemeType::IDENTIFIER, "cpp_files", {4, 57, 152}},
    {LexemeType::FN_END, ")", {4, 66, 161}},
    {LexemeType::NEWLINE, "\n", {4, 67, 162}},

    // Line 5:     { (4 spaces + { + \n)
    {LexemeType::BLOCK_START, "{", {5, 5, 167}},
    {LexemeType::NEWLINE, "\n", {5, 6, 168}},

    // Line 6:         { (8 spaces + { + \n)
    {LexemeType::BLOCK_START, "{", {6, 9, 177}},
    {LexemeType::NEWLINE, "\n", {6, 10, 178}},

    // Line 7:             { (12 spaces + { + \n)
    {LexemeType::BLOCK_START, "{", {7, 13, 191}},
    {LexemeType::NEWLINE, "\n", {7, 14, 192}},

    // Line 8:                 {} (16 spaces + { + } + \n)
    {LexemeType::BLOCK_START, "{", {8, 17, 209}},
    {LexemeType::BLOCK_END, "}", {8, 18, 210}},
    {LexemeType::NEWLINE, "\n", {8, 19, 211}},

    // Line 9:             } (12 spaces + } + \n)
    {LexemeType::BLOCK_END, "}", {9, 13, 224}},
    {LexemeType::NEWLINE, "\n", {9, 14, 225}},

    // Line 10:        } (8 spaces + } + \n)
    {LexemeType::BLOCK_END, "}", {10, 9, 234}},
    {LexemeType::NEWLINE, "\n", {10, 10, 235}},

    // Line 11:    } (4 spaces + } + \n)
    {LexemeType::BLOCK_END, "}", {11, 5, 240}},
    {LexemeType::NEWLINE, "\n", {11, 6, 241}},

    // Line 12: } (0 spaces + } + EOF)
    {LexemeType::BLOCK_END, "}", {12, 1, 242}},
    {LexemeType::END_OF_FILE, "", {12, 2, 243}}};

TEST_CASE("Lex strange data", "[lexer]") {
    Lexer lexer(get_path("LexingEdgeCases.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    REQUIRE(lexemes.size() == strange_data_exp.size());

    for (size_t i = 0; i < strange_data_exp.size(); i++) {
        const Lexeme& exp = strange_data_exp.at(i);
        const Lexeme& got = lexemes.at(i);

        INFO("Comparing '" + got.value + "' with '" + exp.value + "'");
        INFO("Lexeme index: " + std::to_string(i));

        REQUIRE(got.type == exp.type);
        REQUIRE(got.loc == exp.loc);
        REQUIRE(got.value == exp.value);
    }
}