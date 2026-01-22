#include "mock_proc_spawner.hpp"

MockProcSpawner::MockProcSpawner(std::shared_ptr<MockFsGateway> mock_fs)
    : fs(std::move(mock_fs)), run_count(0) {};

int MockProcSpawner::run(std::vector<std::string>& cmd) {
    auto out_prefix = std::ranges::find(cmd, "-o");
    if (out_prefix == cmd.end()) {
        throw std::invalid_argument("Cannot run mock spawner as cmd does not have an output arg");
    }
    out_prefix++;
    if (out_prefix == cmd.end()) {
        throw std::invalid_argument("Cannot run mock spawner as cmd \"-o\" isn't followed by name");
    }

    fs->touch(*out_prefix);

    run_count++;

    return 0;
}

size_t MockProcSpawner::get_run_count() const { return run_count; }
