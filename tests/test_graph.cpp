#include "catch.hpp"
#include "src/dictionaries/rules.hpp"
#include "src/rule_graph.hpp"
#include "utils.hpp"

using AdjList = std::unordered_map<std::string, std::vector<std::string>>;
const AdjList adj_list{{"a", {"b", "c"}}, {"b", {"d"}}, {"c", {"d", "e"}}};

/**
 * The basic tests for single, multi and clean rules all work on the following graph
 *      a
 *     / \
 *    b   c
 *     \ / \
 *      d   e
 */

/**
 * Test the graph above on various metrics.
 * @note I could automate a lot of this process with templates but I dont want to have templates
 * in my tests if possible
 */
void test_basic_graph(const RuleGraph& graph) {
    REQUIRE_FALSE(graph.cyclical_dep_exists());

    REQUIRE(graph.num_rules() == 3);

    REQUIRE(graph.is_rule("b") == true);
    REQUIRE_FALSE(graph.is_rule("e"));

    const std::vector<std::string> a_deps = {"b", "c"};
    const std::vector<std::string> c_deps = {"d", "e"};
    REQUIRE(graph.dependencies("a") == a_deps);
    REQUIRE(graph.dependencies("c") == c_deps);

    const Rule& a_rule = graph.get_rule("a");
    REQUIRE(a_rule.get_name() == "a");
    REQUIRE(a_rule.get_deps() == a_deps);
}

TEST_CASE("Test basic graph with single rules", "[rule_graph]") {
    std::vector<std::unique_ptr<Rule>> rules;
    for (const auto& [v, adj] : adj_list) {
        rules.push_back(std::make_unique<SingleRule>(v, adj, Step::COMPILE, Location{}));
    }
    const RuleGraph graph{std::move(rules)};

    test_basic_graph(graph);
}

TEST_CASE("Test basic graph with multi rules", "[rule_graph]") {
    std::vector<std::unique_ptr<Rule>> rules;
    for (const auto& [v, adj] : adj_list) {
        rules.push_back(std::make_unique<MultiRule>(v, adj, adj, Step::COMPILE, Location{}));
    }
    const RuleGraph graph{std::move(rules)};

    test_basic_graph(graph);
}

TEST_CASE("Test basic graph with clean rules", "[rule_graph]") {
    std::vector<std::unique_ptr<Rule>> rules;
    for (const auto& [v, adj] : adj_list) {
        rules.push_back(std::make_unique<CleanRule>(v, adj, Location{}));
    }
    const RuleGraph graph{std::move(rules)};

    test_basic_graph(graph);
}

TEST_CASE("Test empty graph", "[rule_graph]") {
    const RuleGraph graph{{}};
    REQUIRE_FALSE(graph.cyclical_dep_exists());
    REQUIRE_FALSE(graph.is_rule("fake_rule"));
    REQUIRE(graph.num_rules() == 0);
}

TEST_CASE("Test graph with cyclical dependency", "[rule_graph]") {
    std::vector<std::unique_ptr<Rule>> rules;
    const AdjList adj_list{{"a", {"b", "d"}}, {"b", {"c"}}, {"c", {"d"}}, {"d", {"c"}}};
    for (const auto& [v, adj] : adj_list) {
        rules.push_back(std::make_unique<CleanRule>(v, adj, Location{}));
    }
    const RuleGraph graph{std::move(rules)};

    REQUIRE(graph.cyclical_dep_exists() == true);
}

TEST_CASE("Test error thrown when performing operations on unknown nodes", "[rule_graph]") {
    const RuleGraph graph{{}};

    REQUIRE_THROWS(graph.dependencies("fake"));
    REQUIRE_THROWS(graph.get_rule("fake"));
}