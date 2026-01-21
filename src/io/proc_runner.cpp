#include "proc_runner.hpp"

#include "../errors/error.hpp"
#include "spawn.h"
#include "sys/wait.h"
#include "unistd.h"

int PosixProcRunner::run(std::vector<std::string> cmd) try {
    std::vector<char*> raw_args;
    raw_args.reserve(cmd.size());
    for (std::string& s : cmd) {
        raw_args.push_back(s.data());
    }
    raw_args.push_back(nullptr);

    const char* proc = cmd[0].data();
    pid_t pid;
    int spawn_res = posix_spawnp(&pid, proc, nullptr, nullptr, raw_args.data(), environ);
    if (spawn_res != 0) {
        throw SystemError("Process execution failed for command '" + cmd_str(cmd) + "'");
    }

    int status;
    waitpid(pid, &status, 0);
    if (status != 0) {
        throw SystemError("Process execution failed for command '" + cmd_str(cmd) + "'");
    }

    return status;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Executing command via POSIX spawn");
}

std::string PosixProcRunner::cmd_str(std::vector<std::string>& cmd) const {
    std::string str;
    for (const std::string& tok : cmd) {
        if (!str.empty()) {
            str += ' ';
        }
        str += tok;
    }

    return str;
}
