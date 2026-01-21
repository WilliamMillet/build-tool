#ifndef PROC_RUNNER_H
#define PROC_RUNNER_H

#include <string>
#include <vector>

/** Interface between program and processes. Primarily useful for enabling dependency injection */
class ProcessRunner {
   public:
    virtual int run(std::vector<std::string> cmd);
};

class PosixProcRunner : public ProcessRunner {
   public:
    int run(std::vector<std::string> cmd) override;

   private:
    std::string cmd_str(std::vector<std::string>& cmd) const;
};

#endif