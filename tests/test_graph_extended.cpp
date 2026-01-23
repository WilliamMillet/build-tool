/** Extended tests for rule graph functionality */
#include <memory>
#include <vector>

#include "catch.hpp"
#include "src/dictionaries/rules.hpp"
#include "src/rule_graph.hpp"
#include "utils.hpp"

using AdjList = std::unordered_map<std::string, std::vector<std::string>>;

// Graph Structure Tests

TEST_CASE("Single node graph works correctly", "[rule_graph][structure]") {
    std::vector<std::unique_ptr<Rule>> rules;
    // Need at least one dependency for the rule to be in dep_map
    rules.push_back(std::make_unique<SingleRule>("only", std::vector<std::string>{"file.cpp"},
                                                 Step::COMPILE, Location{}));

    RuleGraph graph{std::move(rules)};

    REQUIRE(graph.num_rules() == 1);
    REQUIRE(graph.is_rule("only"));
    REQUIRE(graph.dependencies("only").size() == 1);
    REQUIRE_FALSE(graph.cyclical_dep_exists());
}

TEST_CASE("Linear chain graph has no cycles", "[rule_graph][structure]") {
    // a -> b -> c -> d (linear chain)
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>("a", std::vector<std::string>{"b"}, Step::COMPILE,
                                                 Location{}));
    rules.push_back(std::make_unique<SingleRule>("b", std::vector<std::string>{"c"}, Step::COMPILE,
                                                 Location{}));
    rules.push_back(std::make_unique<SingleRule>("c", std::vector<std::string>{"d"}, Step::COMPILE,
                                                 Location{}));

    RuleGraph graph{std::move(rules)};

    REQUIRE_FALSE(graph.cyclical_dep_exists());
    REQUIRE(graph.num_rules() == 3);

    // Verify dependencies
    REQUIRE(graph.dependencies("a") == std::vector<std::string>{"b"});
    REQUIRE(graph.dependencies("b") == std::vector<std::string>{"c"});
    REQUIRE(graph.dependencies("c") == std::vector<std::string>{"d"});
}

TEST_CASE("Diamond dependency graph has no cycles", "[rule_graph][structure]") {
    // Diamond: a depends on b and c, both b and c depend on d
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>("a", std::vector<std::string>{"b", "c"},
                                                 Step::LINK, Location{}));
    rules.push_back(std::make_unique<SingleRule>("b", std::vector<std::string>{"d"}, Step::COMPILE,
                                                 Location{}));
    rules.push_back(std::make_unique<SingleRule>("c", std::vector<std::string>{"d"}, Step::COMPILE,
                                                 Location{}));

    RuleGraph graph{std::move(rules)};

    REQUIRE_FALSE(graph.cyclical_dep_exists());
    REQUIRE(graph.num_rules() == 3);
}

TEST_CASE("Self-referential rule creates cycle", "[rule_graph][cycles]") {
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>("a", std::vector<std::string>{"a"}, Step::COMPILE,
                                                 Location{}));

    RuleGraph graph{std::move(rules)};

    REQUIRE(graph.cyclical_dep_exists());
}

TEST_CASE("Two node cycle is detected", "[rule_graph][cycles]") {
    // a -> b -> a
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>("a", std::vector<std::string>{"b"}, Step::COMPILE,
                                                 Location{}));
    rules.push_back(std::make_unique<SingleRule>("b", std::vector<std::string>{"a"}, Step::COMPILE,
                                                 Location{}));

    RuleGraph graph{std::move(rules)};

    REQUIRE(graph.cyclical_dep_exists());
}

TEST_CASE("Cycle in subgraph is detected", "[rule_graph][cycles]") {
    // a -> b -> c -> d -> b (cycle not involving a)
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>("a", std::vector<std::string>{"b"}, Step::COMPILE,
                                                 Location{}));
    rules.push_back(std::make_unique<SingleRule>("b", std::vector<std::string>{"c"}, Step::COMPILE,
                                                 Location{}));
    rules.push_back(std::make_unique<SingleRule>("c", std::vector<std::string>{"d"}, Step::COMPILE,
                                                 Location{}));
    rules.push_back(std::make_unique<SingleRule>("d", std::vector<std::string>{"b"}, Step::COMPILE,
                                                 Location{}));

    RuleGraph graph{std::move(rules)};

    REQUIRE(graph.cyclical_dep_exists());
}

TEST_CASE("Graph with mixed rule types works", "[rule_graph][mixed]") {
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>(
        "app", std::vector<std::string>{"main.o", "utils.o"}, Step::LINK, Location{}));
    rules.push_back(std::make_unique<MultiRule>(
        "compile", std::vector<std::string>{"main.cpp", "utils.cpp"},
        std::vector<std::string>{"main.o", "utils.o"}, Step::COMPILE, Location{}));
    rules.push_back(std::make_unique<CleanRule>(
        "clean", std::vector<std::string>{"app", "main.o", "utils.o"}, Location{}));

    RuleGraph graph{std::move(rules)};

    REQUIRE(graph.num_rules() == 3);
    REQUIRE(graph.is_rule("app"));
    REQUIRE(graph.is_rule("compile"));
    REQUIRE(graph.is_rule("clean"));
}

// Rule Access Tests

TEST_CASE("get_rule returns correct rule", "[rule_graph][access]") {
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>(
        "my_rule", std::vector<std::string>{"dep1", "dep2"}, Step::LINK, Location{}));

    RuleGraph graph{std::move(rules)};

    const Rule& rule = graph.get_rule("my_rule");
    REQUIRE(rule.get_name() == "my_rule");
    REQUIRE(rule.get_deps().size() == 2);
}

TEST_CASE("is_rule distinguishes rules from file dependencies", "[rule_graph][access]") {
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>(
        "build", std::vector<std::string>{"main.cpp", "utils.cpp"}, Step::COMPILE, Location{}));

    RuleGraph graph{std::move(rules)};

    REQUIRE(graph.is_rule("build"));
    REQUIRE_FALSE(graph.is_rule("main.cpp"));  // This is a file, not a rule
    REQUIRE_FALSE(graph.is_rule("utils.cpp"));
}

// Large Graph Tests

TEST_CASE("Graph handles many rules", "[rule_graph][performance]") {
    std::vector<std::unique_ptr<Rule>> rules;

    // Create 100 rules, each depending on the next (all have at least one dep)
    for (int i = 0; i < 100; i++) {
        std::string name = "rule_" + std::to_string(i);
        std::vector<std::string> deps;
        if (i < 99) {
            deps.push_back("rule_" + std::to_string(i + 1));
        } else {
            // Last rule needs a file dependency to be in dep_map
            deps.push_back("final_file.cpp");
        }
        rules.push_back(std::make_unique<SingleRule>(name, deps, Step::COMPILE, Location{}));
    }

    RuleGraph graph{std::move(rules)};

    REQUIRE(graph.num_rules() == 100);
    REQUIRE_FALSE(graph.cyclical_dep_exists());
    REQUIRE(graph.is_rule("rule_0"));
    REQUIRE(graph.is_rule("rule_99"));
}

TEST_CASE("Graph handles rule with many dependencies", "[rule_graph][performance]") {
    std::vector<std::unique_ptr<Rule>> rules;

    // Create one rule with 50 file dependencies
    std::vector<std::string> deps;
    for (int i = 0; i < 50; i++) {
        deps.push_back("file_" + std::to_string(i) + ".cpp");
    }
    rules.push_back(std::make_unique<SingleRule>("mega_rule", deps, Step::LINK, Location{}));

    RuleGraph graph{std::move(rules)};

    REQUIRE(graph.num_rules() == 1);
    REQUIRE(graph.dependencies("mega_rule").size() == 50);
}

// Edge Cases

TEST_CASE("Rule with duplicate dependencies", "[rule_graph][edge_cases]") {
    std::vector<std::unique_ptr<Rule>> rules;
    // Same dependency listed twice
    rules.push_back(std::make_unique<SingleRule>(
        "app", std::vector<std::string>{"main.cpp", "main.cpp"}, Step::COMPILE, Location{}));

    RuleGraph graph{std::move(rules)};

    // Depending on implementation, this might dedupe or keep both
    REQUIRE(graph.num_rules() == 1);
}

TEST_CASE("Rules with same dependency", "[rule_graph][edge_cases]") {
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>("app1", std::vector<std::string>{"common.cpp"},
                                                 Step::COMPILE, Location{}));
    rules.push_back(std::make_unique<SingleRule>("app2", std::vector<std::string>{"common.cpp"},
                                                 Step::COMPILE, Location{}));

    RuleGraph graph{std::move(rules)};

    REQUIRE(graph.num_rules() == 2);
    REQUIRE_FALSE(graph.cyclical_dep_exists());
}
