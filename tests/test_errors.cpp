/** Error handling tests */
#include "../catch.hpp"
#include "src/build_orchestrator.hpp"
#include "src/errors/error.hpp"
#include "src/lexer.hpp"
#include "src/parsing/parser.hpp"
#include "src/variable_evaluator.hpp"
#include "tests/mocks/mock_fs_gateway.hpp"
#include "tests/mocks/mock_proc_spawner.hpp"
#include "utils.hpp"

// Lexer Error Tests

TEST_CASE("Lexer throws on invalid syntax - unclosed string", "[errors][lexer]") {
    REQUIRE_THROWS_AS(Lexer(IO::get_test_file_path("InvalidSyntax.bf")).lex(), Error);
}

TEST_CASE("Lexer throws on non-existent file", "[errors][lexer]") {
    REQUIRE_THROWS(Lexer("non_existent_file.bf").lex());
}

// Parser Error Tests

TEST_CASE("Parser throws on unexpected token", "[errors][parser]") {
    // Create lexemes with invalid sequence
    std::vector<Lexeme> invalid_lexemes = {
        {LexemeType::IDENTIFIER, "x", {1, 1, 0}},
        {LexemeType::IDENTIFIER, "y", {1, 3, 2}},  // Missing equals
        {LexemeType::NEWLINE, "\n", {1, 4, 3}},
        {LexemeType::END_OF_FILE, "", {2, 1, 4}}};

    Parser parser(invalid_lexemes);
    REQUIRE_THROWS(parser.parse());
}

TEST_CASE("Parser handles empty qualifier correctly", "[errors][parser]") {
    std::vector<Lexeme> invalid_lexemes = {
        {LexemeType::DICT_QUALIFIER, "", {1, 1, 0}},  // Empty qualifier
        {LexemeType::IDENTIFIER, "name", {1, 3, 2}}, {LexemeType::BLOCK_START, "{", {1, 8, 7}},
        {LexemeType::NEWLINE, "\n", {1, 9, 8}},      {LexemeType::BLOCK_END, "}", {2, 1, 9}},
        {LexemeType::NEWLINE, "\n", {2, 2, 10}},     {LexemeType::END_OF_FILE, "", {3, 1, 11}}};

    Parser parser(invalid_lexemes);
    // Should throw or handle gracefully depending on implementation
    REQUIRE_THROWS(parser.parse());
}

// Evaluator Error Tests

TEST_CASE("Evaluator throws on cyclical dependencies", "[errors][evaluator]") {
    // a = b, b = a
    auto a_expr = std::make_unique<VarRefExpr>("b");
    auto b_expr = std::make_unique<VarRefExpr>("a");

    // Need a config to not fail earlier
    auto cfg = Factories::create_cfg_dict();

    std::vector<ParsedVariable> vars;
    vars.push_back({"cfg", std::move(cfg), VarCategory::CONFIG, {0, 0, 0}});
    vars.push_back({"a", std::move(a_expr), VarCategory::REGULAR, {1, 0, 0}});
    vars.push_back({"b", std::move(b_expr), VarCategory::REGULAR, {2, 0, 0}});

    VariableEvaluator evaluator(std::move(vars), FuncRegistry{});
    REQUIRE_THROWS(evaluator.evaluate());
}

TEST_CASE("Evaluator throws on undefined variable reference", "[errors][evaluator]") {
    auto ref_expr = std::make_unique<VarRefExpr>("undefined_var");
    auto cfg = Factories::create_cfg_dict();

    std::vector<ParsedVariable> vars;
    vars.push_back({"cfg", std::move(cfg), VarCategory::CONFIG, {0, 0, 0}});
    vars.push_back({"x", std::move(ref_expr), VarCategory::REGULAR, {1, 0, 0}});

    VariableEvaluator evaluator(std::move(vars), FuncRegistry{});
    REQUIRE_THROWS(evaluator.evaluate());
}

TEST_CASE("Evaluator throws when no config provided", "[errors][evaluator]") {
    auto str_expr = std::make_unique<StringExpr>("hello");

    std::vector<ParsedVariable> vars;
    vars.push_back({"x", std::move(str_expr), VarCategory::REGULAR, {0, 0, 0}});

    VariableEvaluator evaluator(std::move(vars), FuncRegistry{});
    REQUIRE_THROWS(evaluator.evaluate());
}

TEST_CASE("Evaluator throws on type mismatch in binary operation", "[errors][evaluator]") {
    // Try to add string + enum (invalid)
    auto left = std::make_unique<StringExpr>("hello");
    auto right = std::make_unique<EnumExpr>("Step", "COMPILE");
    auto bin_expr =
        std::make_unique<BinaryOpExpr>(BinaryOpType::ADD, std::move(left), std::move(right));

    auto cfg = Factories::create_cfg_dict();

    std::vector<ParsedVariable> vars;
    vars.push_back({"cfg", std::move(cfg), VarCategory::CONFIG, {0, 0, 0}});
    vars.push_back({"x", std::move(bin_expr), VarCategory::REGULAR, {1, 0, 0}});

    VariableEvaluator evaluator(std::move(vars), FuncRegistry{});
    REQUIRE_THROWS(evaluator.evaluate());
}

// Integration Error Tests

TEST_CASE("BuildOrchestrator throws on file with missing config", "[errors][integration]") {
    auto fs = std::make_shared<MockFsGateway>();
    auto proc = std::make_shared<MockProcSpawner>(fs);

    REQUIRE_THROWS(BuildOrchestrator(fs, proc, IO::get_test_file_path("MissingConfig.bf")));
}

// Note: run_rule for non-existent rules does not throw, it just does nothing
// This is current source code behavior

// Error Location Tests

TEST_CASE("Error maintains location information", "[errors]") {
    Error err("Test error", Location{10, 5, 100});

    REQUIRE(err.has_loc());
}

TEST_CASE("Error context stack accumulates correctly", "[errors]") {
    Error err("Base error");
    err.add_ctx("Context 1");
    err.add_ctx("Context 2");

    std::string formatted = err.format();

    REQUIRE(formatted.find("Context 1") != std::string::npos);
    REQUIRE(formatted.find("Context 2") != std::string::npos);
}

TEST_CASE("Location EOF detection works correctly", "[errors]") {
    Location eof = Location::eof_loc();
    REQUIRE(eof.is_eof());

    Location normal{1, 1, 0};
    REQUIRE_FALSE(normal.is_eof());
}
