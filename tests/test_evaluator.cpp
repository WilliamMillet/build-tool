#include <memory>
#include <vector>

#include "../catch.hpp"
#include "src/built_in/func_registry.hpp"
#include "src/errors/error.hpp"
#include "src/parsing/expr.hpp"
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
    std::unique_ptr<Expr> compiler = std::make_unique<StringExpr>("clang++");
    const std::string default_rule = "my_rule";
    const std::vector<std::string> c_flags = {"-g", "Werror"};
    const std::vector<std::string> l_flags = {};

    auto cfg = Testing::Factories::create_cfg(std::move(compiler), c_flags, l_flags, default_rule);

    std::vector<ParsedVariable> vars;
    vars.push_back(ParsedVariable{"cfg", std::move(cfg), VarCategory::CONFIG, Location{0, 0, 0}});

    VariableEvaluator evaluator(std::move(vars), FuncRegistry{});

    QualifiedDicts dicts = evaluator.evaluate();

    REQUIRE(dicts.rules.size() == 0);

    REQUIRE(dicts.cfg.get_compiler() == "clang++");
    REQUIRE(dicts.cfg.get_compilation_flags() == c_flags);
    REQUIRE(dicts.cfg.get_link_flags() == l_flags);
    REQUIRE(dicts.cfg.get_default_rule() == default_rule);
}

TEST_CASE("Config can be based on simple variable dependencies", "[variable_evaluator]") {
    std::unique_ptr<VarRefExpr> compiler = std::make_unique<VarRefExpr>("foo");

    auto cfg = Testing::Factories::create_cfg(std::move(compiler), {"-g", "Werror"}, {}, "test");
    auto foo = std::make_unique<StringExpr>("gcc");
    std::vector<ParsedVariable> vars;
    vars.push_back({"cfg", std::move(cfg), VarCategory::CONFIG, Location{0, 0, 0}});
    vars.push_back({"foo", std::move(foo), VarCategory::REGULAR, Location{0, 0, 0}});

    VariableEvaluator evaluator(std::move(vars), FuncRegistry{});

    QualifiedDicts dicts = evaluator.evaluate();

    REQUIRE(dicts.cfg.get_compiler() == "gcc");
}

TEST_CASE("Test complex variable dependency DAG", "[variable_evaluator]") {
    // Create the following dependency graph
    //     a__
    // b _/   \_c
    //   \_______\d

    // d = "Bar"
    // c = d
    // b = "Foo" + d
    // a = b + c

    // d = "Bar"
    // c = "Bar"
    // b = "Foo" + "Bar" = "FooBar"
    // a = "FooBar" + "Bar" = "FooBarBar"

    auto a_l = std::make_unique<VarRefExpr>("b");
    auto a_r = std::make_unique<VarRefExpr>("c");
    auto a_bin = std::make_unique<BinaryOpExpr>(BinaryOpType::ADD, std::move(a_l), std::move(a_r));
    ParsedVariable a{"a", std::move(a_bin), VarCategory::REGULAR, {0, 0, 0}};

    auto b_l = std::make_unique<StringExpr>("Foo");
    auto b_r = std::make_unique<VarRefExpr>("d");
    auto b_bin = std::make_unique<BinaryOpExpr>(BinaryOpType::ADD, std::move(b_l), std::move(b_r));
    ParsedVariable b{"b", std::move(b_bin), VarCategory::REGULAR, {0, 0, 0}};

    ParsedVariable c{"c", std::make_unique<VarRefExpr>("d"), VarCategory::REGULAR, {0, 0, 0}};

    ParsedVariable d{"d", std::make_unique<StringExpr>("Bar"), VarCategory::REGULAR, {0, 0, 0}};

    std::vector<ParsedVariable> vars;
    vars.push_back(std::move(a));
    vars.push_back(std::move(b));
    vars.push_back(std::move(c));
    vars.push_back(std::move(d));

    auto compiler = std::make_unique<VarRefExpr>("a");
    auto cfg = Testing::Factories::create_cfg(std::move(compiler), {}, {}, "");
    vars.push_back(ParsedVariable{"cfg", std::move(cfg), VarCategory::CONFIG, Location{0, 0, 0}});

    VariableEvaluator evaluator(std::move(vars), FuncRegistry());
    QualifiedDicts dicts = evaluator.evaluate();

    REQUIRE(dicts.cfg.get_compiler() == "FooBarBar");
}

TEST_CASE("Rule parses correctly", "[variable_evaluator]") {}

TEST_CASE("MultiRule parses correctly", "[variable_evaluator]") {}

TEST_CASE("Clean rule parses correctly", "[variable_evaluator]") {}

TEST_CASE("Cyclical variable dependency leads to error", "[variable_evaluator]") {}
