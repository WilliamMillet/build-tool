#include <memory>
#include <vector>

#include "../catch.hpp"
#include "src/built_in/func_registry.hpp"
#include "src/dictionaries/rules.hpp"
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

    auto cfg =
        Testing::Factories::create_cfg_dict(std::move(compiler), c_flags, l_flags, default_rule);

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
    auto compiler = std::make_unique<VarRefExpr>("foo");

    auto cfg =
        Testing::Factories::create_cfg_dict(std::move(compiler), {"-g", "Werror"}, {}, "test");
    auto foo = std::make_unique<StringExpr>("gcc");
    std::vector<ParsedVariable> vars;
    vars.push_back({"cfg", std::move(cfg), VarCategory::CONFIG, Location{0, 0, 0}});
    vars.push_back({"foo", std::move(foo), VarCategory::REGULAR, Location{0, 0, 0}});

    VariableEvaluator evaluator(std::move(vars), FuncRegistry{});

    QualifiedDicts dicts = evaluator.evaluate();

    REQUIRE(dicts.cfg.get_name() == "cfg");
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
    auto cfg = Testing::Factories::create_cfg_dict(std::move(compiler), {}, {}, "");
    vars.push_back(ParsedVariable{"cfg", std::move(cfg), VarCategory::CONFIG, Location{0, 0, 0}});

    VariableEvaluator evaluator(std::move(vars), FuncRegistry());
    QualifiedDicts dicts = evaluator.evaluate();

    REQUIRE(dicts.cfg.get_compiler() == "FooBarBar");
}

TEST_CASE("Rule parses correctly", "[variable_evaluator]") {
    std::vector<ParsedVariable> vars;
    vars.push_back({"cfg", Testing::Factories::create_cfg_dict(), VarCategory::CONFIG, {0, 0, 0}});

    auto rule = std::make_unique<DictionaryExpr>();

    std::vector<std::string> dep_strings = {"lexer.cpp", "parser.cpp"};
    std::vector<std::unique_ptr<Expr>> deps;
    for (const std::string& d : dep_strings) {
        deps.push_back(std::make_unique<StringExpr>(d));
    }
    rule->fields_map[RuleFields::DEPS] = std::make_unique<ListExpr>(std::move(deps));

    rule->fields_map[RuleFields::STEP] = std::make_unique<EnumExpr>("Step", "COMPILE");

    vars.push_back({"app", std::move(rule), VarCategory::SINGLE_RULE, {0, 0, 0}});

    VariableEvaluator evaluator(std::move(vars), FuncRegistry());
    QualifiedDicts dicts = evaluator.evaluate();

    REQUIRE(dicts.rules.size() == 1);
    std::unique_ptr<Rule>& evaluated = dicts.rules.at(0);
    REQUIRE(evaluated->get_name() == "app");
    REQUIRE(evaluated->get_deps() == dep_strings);
}

TEST_CASE("MultiRule parses correctly", "[variable_evaluator]") {
    std::vector<ParsedVariable> vars;
    vars.push_back({"cfg", Testing::Factories::create_cfg_dict(), VarCategory::CONFIG, {0, 0, 0}});

    auto rule = std::make_unique<DictionaryExpr>();

    std::vector<std::string> dep_strings = {"file1.cpp", "file2.cpp", "file3.cpp"};
    std::vector<std::unique_ptr<Expr>> deps;
    for (const std::string& d : dep_strings) {
        deps.push_back(std::make_unique<StringExpr>(d));
    }
    rule->fields_map[RuleFields::DEPS] = std::make_unique<ListExpr>(std::move(deps));

    std::vector<std::string> output_strings = {"file1.o", "file2.o", "file3.o"};
    std::vector<std::unique_ptr<Expr>> outputs;
    for (const std::string& o : output_strings) {
        outputs.push_back(std::make_unique<StringExpr>(o));
    }
    rule->fields_map[RuleFields::OUTPUT] = std::make_unique<ListExpr>(std::move(outputs));

    rule->fields_map[RuleFields::STEP] = std::make_unique<EnumExpr>("Step", "COMPILE");

    vars.push_back({"compilation", std::move(rule), VarCategory::MULTI_RULE, {0, 0, 0}});

    VariableEvaluator evaluator(std::move(vars), FuncRegistry());
    QualifiedDicts dicts = evaluator.evaluate();

    REQUIRE(dicts.rules.size() == 1);
    std::unique_ptr<Rule>& evaluated = dicts.rules.at(0);
    REQUIRE(evaluated->get_name() == "compilation");
    REQUIRE(evaluated->get_deps() == dep_strings);
}

TEST_CASE("Clean rule parses correctly", "[variable_evaluator]") {
    std::vector<ParsedVariable> vars;
    vars.push_back({"cfg", Testing::Factories::create_cfg_dict(), VarCategory::CONFIG, {0, 0, 0}});

    auto rule = std::make_unique<DictionaryExpr>();

    std::vector<std::string> target_strings = {"app", "file1.o", "file2.o", "file3.o"};
    std::vector<std::unique_ptr<Expr>> targets;
    for (const std::string& t : target_strings) {
        targets.push_back(std::make_unique<StringExpr>(t));
    }
    rule->fields_map[RuleFields::TARGETS] = std::make_unique<ListExpr>(std::move(targets));

    vars.push_back({"clean", std::move(rule), VarCategory::CLEAN, {0, 0, 0}});

    VariableEvaluator evaluator(std::move(vars), FuncRegistry());
    QualifiedDicts dicts = evaluator.evaluate();

    REQUIRE(dicts.rules.size() == 1);
    std::unique_ptr<Rule>& evaluated = dicts.rules.at(0);
    REQUIRE(evaluated->get_name() == "clean");
    REQUIRE(evaluated->get_deps() == target_strings);
}

TEST_CASE("Cyclical variable dependency leads to error", "[variable_evaluator]") {
    /**
     * a->-b->-c-->-d
     *  \---<--<---/
     */
    std::vector<ParsedVariable> vars;

    std::vector<std::pair<std::string, std::string>> var_setup = {
        {"b", "a"}, {"c", "b"}, {"d", "c"}, {"d", "a"}};

    for (const auto& [v, dep] : var_setup) {
        auto a_expr = std::make_unique<VarRefExpr>(dep);
        ParsedVariable a{v, std::move(a_expr), VarCategory::REGULAR, {0, 0, 0}};
        vars.push_back(std::move(a));
    }

    VariableEvaluator evaluator(std::move(vars), FuncRegistry{});
    REQUIRE_THROWS(evaluator.evaluate());
}
