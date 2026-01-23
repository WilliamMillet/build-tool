#ifndef BUILD_ORCHESTRATOR_H
#define BUILD_ORCHESTRATOR_H

#include <memory>

#include "io/fs_gateway.hpp"
#include "io/proc_spawner.hpp"
#include "rule_runner.hpp"

class BuildOrchestrator {
   public:
    /** Creates the final runner by executing the pipeline from lexing to graph creation */
    BuildOrchestrator(std::shared_ptr<FSGateway> fs, std::shared_ptr<ProcessSpawner> spawner,
                      std::string src_file);

    void setup();

    void run_rule(std::string cmd) const;

   private:
    std::string src_filename;
    std::unique_ptr<RuleRunner> runner;
};

#endif