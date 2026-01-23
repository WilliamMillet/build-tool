#include <memory>

#include "catch.hpp"
#include "mocks/mock_fs_gateway.hpp"
#include "mocks/mock_proc_spawner.hpp"
#include "src/dictionaries/rules.hpp"
#include "src/rule_graph.hpp"
#include "src/rule_runner.hpp"
#include "utils.hpp"

TEST_CASE("Single rule with compile a single, stale dependency", "[rule_runner]") {
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>("prog", std::vector<std::string>{"a.c", "b.c"},
                                                 Step::COMPILE, Location{0, 0, 0}));
    auto graph = std::make_shared<RuleGraph>(std::move(rules));

    auto cfg = std::make_shared<Config>(Config{"cfg", "clang++", {"-Werror", "-Wall"}, {}, "test"});

    auto fs = std::make_shared<MockFsGateway>();

    fs->touch_at("prog", Time::past());
    fs->touch_at("a.c", Time::future());

    auto spawner = std::make_shared<MockProcSpawner>(fs);
    RuleRunner rule_runner(graph, cfg, spawner, fs);

    REQUIRE(spawner->get_run_count() == 0);
    REQUIRE(fs->get_write_count("prog") == 1);
    REQUIRE(fs->get_write_count("a.c") == 1);

    rule_runner.run_rule("prog");

    REQUIRE(spawner->get_run_count() == 1);
    // Initial write + modification
    REQUIRE(fs->get_write_count("prog") == 2);
}

TEST_CASE("Test multirule with a stale dependencies", "[rule_runner]") {
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>("prog", std::vector<std::string>{"a.c", "b.c"},
                                                 Step::COMPILE, Location{0, 0, 0}));
    auto graph = std::make_shared<RuleGraph>(std::move(rules));

    auto cfg = std::make_shared<Config>(Config{"cfg", "clang++", {"-Werror", "-Wall"}, {}, "test"});

    auto fs = std::make_shared<MockFsGateway>();

    fs->touch_at("prog", Time::future());
    fs->touch_at("a.c", Time::past());
    fs->touch_at("b.c", Time::past());

    auto spawner = std::make_shared<MockProcSpawner>(fs);
    RuleRunner rule_runner(graph, cfg, spawner, fs);

    REQUIRE(spawner->get_run_count() == 0);
    REQUIRE(fs->get_write_count("prog") == 1);
    REQUIRE(fs->get_write_count("a.c") == 1);

    rule_runner.run_rule("prog");

    REQUIRE(spawner->get_run_count() == 0);
    // No change
    REQUIRE(fs->get_write_count("prog") == 1);
}

TEST_CASE("Test multirule with a non-stale dependencies", "[rule_runner]") {
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<MultiRule>("prog", std::vector<std::string>{"a.c", "b.c"},
                                                std::vector<std::string>{"a.o", "b.o"},
                                                Step::COMPILE, Location{0, 0, 0}));
    auto graph = std::make_shared<RuleGraph>(std::move(rules));

    auto cfg = std::make_shared<Config>(Config{"cfg", "clang++", {"-Werror", "-Wall"}, {}, "test"});

    auto fs = std::make_shared<MockFsGateway>();

    fs->touch_at("prog", Time::past());
    fs->touch_at("a.c", Time::future());
    fs->touch_at("b.c", Time::future());

    auto spawner = std::make_shared<MockProcSpawner>(fs);
    RuleRunner rule_runner(graph, cfg, spawner, fs);

    REQUIRE(spawner->get_run_count() == 0);

    rule_runner.run_rule("prog");

    REQUIRE(spawner->get_run_count() == 2);
    REQUIRE(fs->exists("a.o"));
    REQUIRE(fs->exists("b.o"));
}

TEST_CASE("Single rule with compile a single, non-stale dependency", "[rule_runner]") {
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>("prog", std::vector<std::string>{"a.c", "b.c"},
                                                 Step::COMPILE, Location{0, 0, 0}));
    auto graph = std::make_shared<RuleGraph>(std::move(rules));

    auto cfg = std::make_shared<Config>(Config{"cfg", "clang++", {"-Werror", "-Wall"}, {}, "test"});

    auto fs = std::make_shared<MockFsGateway>();

    fs->touch_at("prog", Time::future());
    fs->touch_at("a.c", Time::past());
    fs->touch_at("b.c", Time::past());

    REQUIRE(fs->get_write_count("prog") == 1);

    auto spawner = std::make_shared<MockProcSpawner>(fs);
    RuleRunner rule_runner(graph, cfg, spawner, fs);

    REQUIRE(spawner->get_run_count() == 0);

    rule_runner.run_rule("prog");

    REQUIRE(spawner->get_run_count() == 0);
    // No changes
    REQUIRE(fs->get_write_count("prog") == 1);
}

TEST_CASE("Test clean rule", "[rule_runner]") {
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<CleanRule>(
        "clean", std::vector<std::string>{"a.o", "b.o", "prog"}, Location{0, 0, 0}));
    auto graph = std::make_shared<RuleGraph>(std::move(rules));

    auto cfg = std::make_shared<Config>(Config{"cfg", "clang++", {"-Werror", "-Wall"}, {}, "test"});

    auto fs = std::make_shared<MockFsGateway>();

    const Rule& clean_rule = graph->get_rule("clean");
    REQUIRE(clean_rule.should_run(*fs) == true);

    auto commands = clean_rule.get_commands(*cfg);
    REQUIRE(commands.size() == 1);
    REQUIRE(commands[0][0] == "rm");
    // rm + 3 targets
    REQUIRE(commands[0].size() == 4);
}

TEST_CASE("Non-existant rule leads to error being thrown", "[rule_runner]") {
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>("prog", std::vector<std::string>{"a.c"},
                                                 Step::COMPILE, Location{0, 0, 0}));
    auto graph = std::make_shared<RuleGraph>(std::move(rules));

    auto cfg = std::make_shared<Config>(Config{"cfg", "clang++", {"-Werror", "-Wall"}, {}, "test"});

    auto fs = std::make_shared<MockFsGateway>();
    auto spawner = std::make_shared<MockProcSpawner>(fs);
    RuleRunner rule_runner(graph, cfg, spawner, fs);

    REQUIRE_THROWS_AS(rule_runner.run_rule("nonexistent_rule"), LogicError);
}

TEST_CASE("Single rule chained command construction. No existing files", "[rule_runner]") {
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>("prog", std::vector<std::string>{"main.o"},
                                                 Step::LINK, Location{0, 0, 0}));
    rules.push_back(std::make_unique<SingleRule>("main.o", std::vector<std::string>{"main.c"},
                                                 Step::COMPILE, Location{0, 0, 0}));
    auto graph = std::make_shared<RuleGraph>(std::move(rules));

    auto cfg = std::make_shared<Config>(Config{"cfg", "g++", {"-O2"}, {"-lpthread"}, "test"});

    auto fs = std::make_shared<MockFsGateway>();
    fs->touch_at("main.c", Time::past());

    auto spawner = std::make_shared<MockProcSpawner>(fs);
    RuleRunner rule_runner(graph, cfg, spawner, fs);

    REQUIRE(spawner->get_run_count() == 0);

    rule_runner.run_rule("prog");

    REQUIRE(spawner->get_run_count() == 2);
    REQUIRE(fs->exists("main.o"));
    REQUIRE(fs->exists("prog"));
}

TEST_CASE("Single rule chained command construction. No recompile needed", "[rule_runner]") {
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(std::make_unique<SingleRule>("prog", std::vector<std::string>{"main.o"},
                                                 Step::LINK, Location{0, 0, 0}));
    rules.push_back(std::make_unique<SingleRule>("main.o", std::vector<std::string>{"main.c"},
                                                 Step::COMPILE, Location{0, 0, 0}));
    auto graph = std::make_shared<RuleGraph>(std::move(rules));

    auto cfg = std::make_shared<Config>(Config{"cfg", "g++", {"-O2"}, {"-lpthread"}, "test"});

    auto fs = std::make_shared<MockFsGateway>();

    fs->touch_at("main.c", Time::past());
    fs->touch_at("main.o", Time::future());
    fs->touch_at("prog", Time::future());

    REQUIRE(fs->get_write_count("main.o") == 1);
    REQUIRE(fs->get_write_count("prog") == 1);

    auto spawner = std::make_shared<MockProcSpawner>(fs);
    RuleRunner rule_runner(graph, cfg, spawner, fs);

    REQUIRE(spawner->get_run_count() == 0);

    rule_runner.run_rule("prog");

    // Should not change
    REQUIRE(spawner->get_run_count() == 0);
    REQUIRE(fs->get_write_count("main.o") == 1);
    REQUIRE(fs->get_write_count("prog") == 1);
}
