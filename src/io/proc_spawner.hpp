#ifndef proc_spawner_H
#define proc_spawner_H

#include <string>
#include <vector>

/** Interface between program and processes. Primarily useful for enabling dependency injection */
class ProcessSpawner {
   public:
    virtual ~ProcessSpawner() = default;

    /** Use a command to run a process and forward the return value */
    virtual int run(std::vector<std::string>& cmd) = 0;
};

class PosixProcSpawner : public ProcessSpawner {
   public:
    int run(std::vector<std::string>& cmd) override;

   private:
    std::string cmd_str(const std::vector<std::string>& cmd) const;
};

#endif