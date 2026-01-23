

#ifndef MOCK_PROC_SPAWNER_H
#define MOCK_PROC_SPAWNER_H

#include <memory>
#include <string>
#include <vector>

#include "mock_fs_gateway.hpp"
#include "src/io/proc_spawner.hpp"

/** @note Only supports compilation as of now */
class MockProcSpawner : public ProcessSpawner {
   public:
    MockProcSpawner(std::shared_ptr<MockFsGateway> mock_fs);

    int run(std::vector<std::string>& cmd) override;

    size_t get_run_count() const;

   private:
    std::shared_ptr<MockFsGateway> fs;
    /** Number of times the run method has been used */
    size_t run_count;
};

#endif