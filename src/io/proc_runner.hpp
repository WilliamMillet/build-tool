#include <string>
#include <vector>

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