#include <vector>

#include "../catch.hpp"
#include "../src/lexer.hpp"
#include "src/errors/error.hpp"
#include "utils.hpp"

TEST_CASE("Empty file has only a single EOF lexeme", "[lexer]") {
    Lexer lexer(IO::get_test_file_path("Empty.bf"));
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
    Lexer lexer(IO::get_test_file_path("SimpleVariables.bf"));
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
    Lexer lexer(IO::get_test_file_path("SimpleDicts.bf"));
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
    Lexer lexer(IO::get_test_file_path("LexingEdgeCases.bf"));
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

// Tests for lexeme edge cases

TEST_CASE("Lexer handles multiple newlines", "[lexer][edge_cases]") {
    Lexer lexer(IO::get_test_file_path("LexingEdgeCases.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    // Should not crash and should produce valid output
    REQUIRE(!lexemes.empty());
    REQUIRE(lexemes.back().type == LexemeType::END_OF_FILE);
}

TEST_CASE("Lexer strips comments correctly", "[lexer][comments]") {
    // Comments should not appear in lexeme output
    Lexer lexer(IO::get_test_file_path("SimpleVariables.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    for (const auto& lex : lexemes) {
        // No lexeme should contain comment text
        REQUIRE(lex.value.find("Should be ignored") == std::string::npos);
    }
}

// Tests for lexeme types

TEST_CASE("Lexer identifies all token types", "[lexer][types]") {
    Lexer lexer(IO::get_test_file_path("SimpleDicts.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    bool has_qualifier = false;
    bool has_identifier = false;
    bool has_block_start = false;
    bool has_block_end = false;
    bool has_equals = false;
    bool has_string = false;
    bool has_newline = false;
    bool has_eof = false;

    for (const auto& lex : lexemes) {
        switch (lex.type) {
            case LexemeType::DICT_QUALIFIER:
                has_qualifier = true;
                break;
            case LexemeType::IDENTIFIER:
                has_identifier = true;
                break;
            case LexemeType::BLOCK_START:
                has_block_start = true;
                break;
            case LexemeType::BLOCK_END:
                has_block_end = true;
                break;
            case LexemeType::EQUALS:
                has_equals = true;
                break;
            case LexemeType::STRING:
                has_string = true;
                break;
            case LexemeType::NEWLINE:
                has_newline = true;
                break;
            case LexemeType::END_OF_FILE:
                has_eof = true;
                break;
            default:
                break;
        }
    }

    REQUIRE(has_qualifier);
    REQUIRE(has_identifier);
    REQUIRE(has_block_start);
    REQUIRE(has_block_end);
    REQUIRE(has_equals);
    REQUIRE(has_string);
    REQUIRE(has_newline);
    REQUIRE(has_eof);
}

TEST_CASE("Lexer handles scope resolution operator", "[lexer][operators]") {
    Lexer lexer(IO::get_test_file_path("SimpleVariables.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    bool has_scope_resolver = false;
    for (const auto& lex : lexemes) {
        if (lex.type == LexemeType::SCOPE_RESOLVER && lex.value == "::") {
            has_scope_resolver = true;
            break;
        }
    }

    REQUIRE(has_scope_resolver);
}

TEST_CASE("Lexer handles list delimiters", "[lexer][lists]") {
    Lexer lexer(IO::get_test_file_path("SimpleVariables.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    bool has_list_start = false;
    bool has_list_end = false;
    bool has_delimiter = false;

    for (const auto& lex : lexemes) {
        if (lex.type == LexemeType::LIST_START) has_list_start = true;
        if (lex.type == LexemeType::LIST_END) has_list_end = true;
        if (lex.type == LexemeType::DELIMETER) has_delimiter = true;
    }

    REQUIRE(has_list_start);
    REQUIRE(has_list_end);
    REQUIRE(has_delimiter);
}

// Lex location tests

TEST_CASE("Lexer tracks line numbers correctly", "[lexer][location]") {
    Lexer lexer(IO::get_test_file_path("SimpleVariables.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    // Find lexemes from different lines
    size_t max_line = 0;
    for (const auto& lex : lexemes) {
        if (lex.loc.line_no > max_line) {
            max_line = lex.loc.line_no;
        }
    }

    // Should have lexemes from multiple lines
    REQUIRE(max_line >= 2);
}

TEST_CASE("Lexer tracks column numbers correctly", "[lexer][location]") {
    Lexer lexer(IO::get_test_file_path("SimpleVariables.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    // First lexeme on a line should have low column number
    // Subsequent lexemes should have higher column numbers
    bool found_different_columns = false;
    size_t last_col = 0;

    for (const auto& lex : lexemes) {
        if (lex.loc.line_no == 1) {  // Just check first line
            if (lex.loc.col_no > last_col) {
                found_different_columns = true;
            }
            last_col = lex.loc.col_no;
        }
    }

    REQUIRE(found_different_columns);
}

TEST_CASE("Lexer tracks file index correctly", "[lexer][location]") {
    Lexer lexer(IO::get_test_file_path("SimpleVariables.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    // File index should increase monotonically
    size_t last_idx = 0;
    for (const auto& lex : lexemes) {
        REQUIRE(lex.loc.file_idx >= last_idx);
        last_idx = lex.loc.file_idx;
    }
}

// Tests for string lexing

TEST_CASE("Lexer preserves string content", "[lexer][strings]") {
    Lexer lexer(IO::get_test_file_path("SimpleVariables.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    bool found_apple = false;
    bool found_dog = false;
    bool found_cat = false;

    for (const auto& lex : lexemes) {
        if (lex.type == LexemeType::STRING) {
            if (lex.value == "Apple") found_apple = true;
            if (lex.value == "Dog") found_dog = true;
            if (lex.value == "Cat") found_cat = true;
        }
    }

    REQUIRE(found_apple);
    REQUIRE(found_dog);
    REQUIRE(found_cat);
}

// Tests for lexeme identifiers

TEST_CASE("Lexer identifies identifiers correctly", "[lexer][identifiers]") {
    Lexer lexer(IO::get_test_file_path("SimpleVariables.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    bool found_word = false;
    bool found_array = false;
    bool found_enum = false;
    bool found_step = false;
    bool found_link = false;

    for (const auto& lex : lexemes) {
        if (lex.type == LexemeType::IDENTIFIER) {
            if (lex.value == "word") found_word = true;
            if (lex.value == "array") found_array = true;
            if (lex.value == "enum") found_enum = true;
            if (lex.value == "Step") found_step = true;
            if (lex.value == "LINK") found_link = true;
        }
    }

    REQUIRE(found_word);
    REQUIRE(found_array);
    REQUIRE(found_enum);
    REQUIRE(found_step);
    REQUIRE(found_link);
}

// Test for qualified lexemes

TEST_CASE("Lexer identifies qualifiers correctly", "[lexer][qualifiers]") {
    Lexer lexer(IO::get_test_file_path("SimpleDicts.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    std::vector<std::string> found_qualifiers;
    for (const auto& lex : lexemes) {
        if (lex.type == LexemeType::DICT_QUALIFIER) {
            found_qualifiers.push_back(lex.value);
        }
    }

    // Should find at least one qualifier
    REQUIRE(!found_qualifiers.empty());
}

// TEsts for lexeme sequences

TEST_CASE("Assignment lexemes are in correct order", "[lexer][sequence]") {
    Lexer lexer(IO::get_test_file_path("SimpleVariables.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    // Find an assignment sequence: IDENTIFIER EQUALS value
    for (size_t i = 0; i + 2 < lexemes.size(); i++) {
        if (lexemes[i].type == LexemeType::IDENTIFIER &&
            lexemes[i + 1].type == LexemeType::EQUALS) {
            // Next token should be a value (STRING, LIST_START, or IDENTIFIER for enum/varref)
            LexemeType next_type = lexemes[i + 2].type;
            bool is_value_start =
                (next_type == LexemeType::STRING || next_type == LexemeType::LIST_START ||
                 next_type == LexemeType::IDENTIFIER);
            REQUIRE(is_value_start);
            break;
        }
    }
}

TEST_CASE("Dictionary lexemes have matching braces", "[lexer][sequence]") {
    Lexer lexer(IO::get_test_file_path("SimpleDicts.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    int brace_count = 0;
    for (const auto& lex : lexemes) {
        if (lex.type == LexemeType::BLOCK_START) brace_count++;
        if (lex.type == LexemeType::BLOCK_END) brace_count--;

        // Should never go negative
        REQUIRE(brace_count >= 0);
    }

    // Should be balanced
    REQUIRE(brace_count == 0);
}

TEST_CASE("List lexemes have matching brackets", "[lexer][sequence]") {
    Lexer lexer(IO::get_test_file_path("SimpleVariables.bf"));
    std::vector<Lexeme> lexemes = lexer.lex();

    int bracket_count = 0;
    for (const auto& lex : lexemes) {
        if (lex.type == LexemeType::LIST_START) bracket_count++;
        if (lex.type == LexemeType::LIST_END) bracket_count--;

        REQUIRE(bracket_count >= 0);
    }

    REQUIRE(bracket_count == 0);
}
