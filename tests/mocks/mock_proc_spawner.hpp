

#include <string>
#include <vector>

#include "mock_fs_gateway.hpp"
#include "src/io/proc_spawner.hpp"

class MockProcSpawner : public ProcessRunner {
   public:
    MockProcSpawner(MockFsGateway* mock_fs);

    int run(std::vector<std::string> cmd) override;

   private:
    MockFsGateway* fs;
};
