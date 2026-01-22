#include "mock_proc_spawner.hpp"

MockProcSpawner::MockProcSpawner(MockFsGateway* mock_fs) : fs(mock_fs) {};

int MockProcSpawner::run(std::vector<std::string> cmd) {
    auto out_prefix = std::ranges::find(cmd, "-o");
    if (out_prefix == cmd.end()) {
        throw std::invalid_argument("Cannot run mock spawner as cmd does not have an output arg");
    }
    out_prefix++;
    if (out_prefix == cmd.end()) {
        throw std::invalid_argument("Cannot run mock spawner as cmd \"-o\" isn't followed by name");
    }

    fs->touch(*out_prefix);

    return 0;
}