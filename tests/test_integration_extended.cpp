/** Extended integration tests for the build system */
#include "catch.hpp"
#include "src/build_orchestrator.hpp"
#include "tests/mocks/mock_fs_gateway.hpp"
#include "tests/mocks/mock_proc_spawner.hpp"
#include "utils.hpp"

// MultiRule Integration Tests

TEST_CASE("MultiRule file compiles multiple sources correctly", "[integration][multirule]") {
    auto fs = std::make_shared<MockFsGateway>();
    fs->touch_at("main.cpp", Time::past());
    fs->touch_at("utils.cpp", Time::past());
    fs->touch_at("parser.cpp", Time::past());

    auto proc = std::make_shared<MockProcSpawner>(fs);

    BuildOrchestrator orchestrator(fs, proc, IO::get_test_file_path("MultiRuleValid.bf"));

    REQUIRE(proc->get_run_count() == 0);

    orchestrator.run_rule("app");

    // Only runs the app link command, not the compilation multirule
    REQUIRE(proc->get_run_count() == 1);
}

TEST_CASE("MultiRule only recompiles changed files", "[integration][multirule]") {
    auto fs = std::make_shared<MockFsGateway>();

    // All object files are newer than sources except one
    fs->touch_at("main.cpp", Time::past());
    fs->touch_at("utils.cpp", Time::past());
    fs->touch_at("parser.cpp", Time::past());
    fs->touch_at("main", Time::future());
    fs->touch_at("utils", Time::future());
    fs->touch_at("parser", Time::future());
    fs->touch_at("app", Time::past());  // App is older, needs rebuild

    auto proc = std::make_shared<MockProcSpawner>(fs);

    BuildOrchestrator orchestrator(fs, proc, IO::get_test_file_path("MultiRuleValid.bf"));

    orchestrator.run_rule("app");

    // Only app needs to be relinked since all objs are newer
    REQUIRE(proc->get_run_count() == 1);
}

TEST_CASE("No recompilation when files are up to date", "[integration][incremental]") {
    auto fs = std::make_shared<MockFsGateway>();

    // All dependencies are older than output
    fs->touch_at("a.cpp", Time::past());
    fs->touch_at("app", Time::future());

    auto proc = std::make_shared<MockProcSpawner>(fs);

    BuildOrchestrator orchestrator(fs, proc, IO::get_test_file_path("SimpleValid.bf"));

    orchestrator.run_rule("app");

    // Nothing should be recompiled
    REQUIRE(proc->get_run_count() == 0);
}

TEST_CASE("Recompilation triggered when source is newer than output",
          "[integration][incremental]") {
    auto fs = std::make_shared<MockFsGateway>();

    // Source is newer than output
    fs->touch_at("a.cpp", Time::future());
    fs->touch_at("app", Time::past());

    auto proc = std::make_shared<MockProcSpawner>(fs);

    BuildOrchestrator orchestrator(fs, proc, IO::get_test_file_path("SimpleValid.bf"));

    orchestrator.run_rule("app");

    // Should recompile
    REQUIRE(proc->get_run_count() == 1);
}

TEST_CASE("Build runs when output file doesn't exist", "[integration][incremental]") {
    auto fs = std::make_shared<MockFsGateway>();

    // Only source exists, no output
    fs->touch_at("a.cpp", Time::past());
    // app doesn't exist

    auto proc = std::make_shared<MockProcSpawner>(fs);

    BuildOrchestrator orchestrator(fs, proc, IO::get_test_file_path("SimpleValid.bf"));

    orchestrator.run_rule("app");

    // Should compile since app doesn't exist
    REQUIRE(proc->get_run_count() == 1);
}

// Multiple Rule Runs

TEST_CASE("Running same rule twice doesn't recompile if unchanged",
          "[integration][multiple_runs]") {
    auto fs = std::make_shared<MockFsGateway>();
    fs->touch_at("a.cpp", Time::past());

    auto proc = std::make_shared<MockProcSpawner>(fs);

    BuildOrchestrator orchestrator(fs, proc, IO::get_test_file_path("SimpleValid.bf"));

    // First run - should compile
    orchestrator.run_rule("app");
    REQUIRE(proc->get_run_count() == 1);

    // Second run - should not recompile since app was just created
    orchestrator.run_rule("app");
    REQUIRE(proc->get_run_count() == 1);  // Still 1, no new compilation
}

// Default Rule Tests

TEST_CASE("Default rule is used when no rule specified", "[integration][default]") {
    auto fs = std::make_shared<MockFsGateway>();
    fs->touch_at("a.cpp", Time::past());

    auto proc = std::make_shared<MockProcSpawner>(fs);

    BuildOrchestrator orchestrator(fs, proc, IO::get_test_file_path("SimpleValid.bf"));

    // Run default rule (which should be "app" according to config)
    orchestrator.run_rule("app");

    REQUIRE(proc->get_run_count() == 1);
    REQUIRE(fs->exists("app"));
}

// String Concatenation Tests

TEST_CASE("String concatenation works in build files", "[integration][string_concat]") {
    auto fs = std::make_shared<MockFsGateway>();
    fs->touch_at("src/main.cpp", Time::past());

    auto proc = std::make_shared<MockProcSpawner>(fs);

    BuildOrchestrator orchestrator(fs, proc, IO::get_test_file_path("StringConcat.bf"));

    REQUIRE_NOTHROW(orchestrator.run_rule("app"));
}

// List Concatenation Tests

TEST_CASE("List concatenation works correctly", "[integration][list_concat]") {
    auto fs = std::make_shared<MockFsGateway>();
    fs->touch_at("a", Time::past());
    fs->touch_at("b", Time::past());
    fs->touch_at("c", Time::past());
    fs->touch_at("d", Time::past());

    auto proc = std::make_shared<MockProcSpawner>(fs);

    BuildOrchestrator orchestrator(fs, proc, IO::get_test_file_path("NestedLists.bf"));

    REQUIRE_NOTHROW(orchestrator.run_rule("app"));
}
