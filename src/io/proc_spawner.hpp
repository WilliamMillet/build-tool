#ifndef proc_spawner_H
#define proc_spawner_H

#include <string>
#include <vector>

/** Interface between program and processes. Primarily useful for enabling dependency injection */
class ProcessSpawner {
   public:
    virtual ~ProcessSpawner() = default;

    /**
     * @brief Use a command to run a process and forward the return value
     *
     * @param cmd The command tokens to run (e.g. {'g++', 'app.cpp', '-o', 'app})
     * @return int The process return value
     * @throws If there is an error spawning the process
     */
    virtual int run(std::vector<std::string>& cmd) = 0;
};

class PosixProcSpawner : public ProcessSpawner {
   public:
    int run(std::vector<std::string>& cmd) override;

   private:
    /** Join the command into a single space separated string */
    std::string cmd_str(const std::vector<std::string>& cmd) const;
};

#endif