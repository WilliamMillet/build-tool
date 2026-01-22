#include <stdexcept>

#include "build_orchestrator.hpp"
#include "io/fs_gateway.hpp"
#include "io/proc_spawner.hpp"

int main(int argc, char** argv) {
    if (argc > 2) {
        std::string use_one = "<" + std::string(argv[0]) + "> <file>";
        throw std::invalid_argument("Invalid CLI arguments. Usage:\n <" + use_one);
    }

    const std::string src = argv[1];
    BuildOrchestrator orchestrator(std::make_shared<ProdFSGateway>(),
                                   std::make_shared<PosixProcSpawner>(), src);
    for (int i = 2; i < argc; i++) {
        orchestrator.run_command(argv[i]);
    }
}