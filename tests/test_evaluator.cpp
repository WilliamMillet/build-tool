#include <vector>

#include "../catch.hpp"
#include "src/errors/error.hpp"
#include "src/parsing/parser.hpp"
#include "src/variable_evaluator.hpp"
#include "utils.hpp"

TEST_CASE("Test error thrown with empty file", "[variable_evaluator]") {
    VariableEvaluator evaluator({}, FuncRegistry{});

    REQUIRE_THROWS(evaluator.evaluate());
}

TEST_CASE("Test error thrown with no config file", "[variable_evaluator]") {
    std::vector<ParsedVariable> vars;
    vars.push_back(ParsedVariable{"foo", std::make_unique<StringExpr>("bar"), VarCategory::REGULAR,
                                  Location{0, 0, 0}});

    VariableEvaluator evaluator(std::move(vars), FuncRegistry{});

    REQUIRE_THROWS(evaluator.evaluate());
}

TEST_CASE("Config only with no variables evaluates correctly", "[variable_evaluator]") {
    const std::string compiler = "clang++";
    const std::string default_rule = "my_rule";
    const std::vector<std::string> c_flags = {"-g", "Werror"};
    const std::vector<std::string> l_flags = {};

    std::unique_ptr<Expr> expr =
        TestUtils::ExprFactories::create_cfg(compiler, c_flags, l_flags, default_rule);

    std::vector<ParsedVariable> vars;
    vars.push_back(ParsedVariable{"cfg", std::move(expr), VarCategory::CONFIG, Location{0, 0, 0}});

    VariableEvaluator evaluator(std::move(vars), FuncRegistry{});

    QualifiedDicts dicts = evaluator.evaluate();

    REQUIRE(dicts.rules.size() == 0);

    REQUIRE(dicts.cfg.get_compiler() == compiler);
    REQUIRE(dicts.cfg.get_compilation_flags() == c_flags);
    REQUIRE(dicts.cfg.get_link_flags() == l_flags);
    REQUIRE(dicts.cfg.get_default_rule() == default_rule);
}

TEST_CASE("Config can be based on simple variable dependencies", "[variable_evaluator]") {}

TEST_CASE("Test complex variable dependency DAG", "[variable_evaluator]") {}

TEST_CASE("Rule parses correctly", "[variable_evaluator]") {}

TEST_CASE("MultiRule parses correctly", "[variable_evaluator]") {}

TEST_CASE("Clean rule parses correctly", "[variable_evaluator]") {}

TEST_CASE("Cyclical variable dependency leads to error", "[variable_evaluator]") {}
