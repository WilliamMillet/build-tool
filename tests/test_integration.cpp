/** Integration tests for the entire system */
#include "catch.hpp"
#include "src/build_orchestrator.hpp"
#include "tests/mocks/mock_fs_gateway.hpp"
#include "tests/mocks/mock_proc_spawner.hpp"
#include "utils.hpp"

TEST_CASE("No errors occur when running a simple file", "[integration]") {
    auto fs = std::make_shared<MockFsGateway>();
    fs->touch_at("a.cpp", Time::past());

    auto proc = std::make_shared<MockProcSpawner>(fs);

    BuildOrchestrator orchestrator(fs, proc, IO::get_test_file_path("SimpleValid.bf"));

    orchestrator.run_rule("app");
}

TEST_CASE("Simple file does the correct number of writes", "[integration]") {
    auto fs = std::make_shared<MockFsGateway>();
    fs->touch_at("a.cpp", Time::past());
    REQUIRE(fs->get_write_count("a.cpp") == 1);

    auto p_runner = std::make_shared<MockProcSpawner>(fs);

    BuildOrchestrator orchestrator(fs, p_runner, IO::get_test_file_path("SimpleValid.bf"));

    orchestrator.run_rule("app");

    REQUIRE(p_runner->get_run_count() == 1);
    REQUIRE(fs->get_write_count("a.cpp") == 1);
    REQUIRE(fs->get_write_count("app") == 1);
}
