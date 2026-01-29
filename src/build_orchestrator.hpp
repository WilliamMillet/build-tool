#ifndef BUILD_ORCHESTRATOR_H
#define BUILD_ORCHESTRATOR_H

#include <memory>

#include "io/fs_gateway.hpp"
#include "io/proc_spawner.hpp"
#include "rule_runner.hpp"

/**
 * @brief A controller for managing orchestrating all steps of the build process
 *
 */
class BuildOrchestrator {
   public:
    /**
     * @brief Construct a new Build Orchestrator object
     *
     * @param fs The file system abstraction that all FS interactions will occur through
     * @param spawner The process spawning abstraction that all processes will be spawned wth
     * @param src_file The file containing the build configuration
     */
    BuildOrchestrator(std::shared_ptr<FSGateway> fs, std::shared_ptr<ProcessSpawner> spawner,
                      std::string src_file);

    /**
     * @brief Perform all pre-processing that must occur before command execution
     *
     */
    void setup();

    /**
     * @brief Execute a command
     *
     * @param cmd As of now, a command is just an identifier to a rule
     */
    void run_rule(std::string cmd) const;

   private:
    std::string src_filename;
    std::unique_ptr<RuleRunner> runner;
};

#endif