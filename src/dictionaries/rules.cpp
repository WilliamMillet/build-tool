#include "rules.hpp"

#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <string>

#include "../errors/error.hpp"
#include "config.hpp"
#include "spawn.h"
#include "sys/wait.h"
#include "unistd.h"

namespace fs = std::filesystem;

Rule::Rule(std::string _qualifier, std::string _name, std::vector<std::string> _deps, Location _loc)
    : qualifier("<" + _qualifier + ">"), name(_name), deps(_deps), loc(_loc) {};

const std::vector<std::string>& Rule::get_deps() const { return deps; };
const std::string& Rule::get_name() const { return name; };
const Location& Rule::get_loc() const { return loc; };

void Rule::try_compile(std::vector<std::string>& cmd, const Config& cfg) const try {
    std::vector<char*> raw_args;
    raw_args.reserve(cmd.size());
    for (std::string& s : cmd) {
        raw_args.push_back(s.data());
    }

    const char* compiler_arr = cfg.get_compiler().data();
    pid_t proc;
    int spawn_res = posix_spawnp(&proc, compiler_arr, nullptr, nullptr, raw_args.data(), environ);
    if (spawn_res != 0) {
        throw SystemError("Compilation failed for '" + qualifier + " " + name + "'");
    }

    int status;
    waitpid(proc, &status, 0);
    if (status != 0) {
        throw SystemError("Compilation failed for '" + qualifier + " " + name + "'");
    }
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Compiling", loc);
}

bool Rule::has_updated_dep() const {
    if (!fs::exists(name)) return true;

    auto target_write_t = fs::last_write_time(name);
    return std::ranges::any_of(deps, [&](std::string d) {
        return !fs::exists(d) || fs::last_write_time(d) >= target_write_t;
    });
}

SingleRule::SingleRule(std::string _name, std::vector<std::string> deps, Step _step,
                       Location _loc) try
    : Rule("Rule", std::move(_name), std::move(deps), _loc), step(_step) {
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Constructing '<Rule> " + _name + "'", _loc);
}

bool SingleRule::should_run() const { return has_updated_dep(); }

void SingleRule::run(const Config& cfg) const try {
    std::vector<std::string> cmd = {cfg.get_compiler()};

    auto& flags = (step == Step::COMPILE) ? cfg.get_compilation_flags() : cfg.get_link_flags();
    for (const std::string& flag : flags) {
        cmd.push_back(flag);
    }

    for (const std::string& dep : deps) {
        cmd.push_back(dep);
    }

    cmd.push_back("-o");
    cmd.push_back(name);

    try_compile(cmd, cfg);
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Running '<Rule> " + name + "'", get_loc());
}

MultiRule::MultiRule(std::string _name, std::vector<std::string> _deps,
                     std::vector<std::string> _out, Step _step, Location _loc) try
    : Rule("MultiRule", std::move(_name), std::move(_deps), _loc), output(_out), step(_step) {
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Constructing '<MultiRule> " + _name + "'", _loc);
}

bool MultiRule::should_run() const { return has_updated_dep(); }

void MultiRule::run(const Config& cfg) const try {
    // Invariant deps.size() == output.size() should be enforced in the constructor
    for (size_t i = 0; i < deps.size(); i++) {
        std::vector<std::string> cmd = {cfg.get_compiler()};
        auto& flags = (step == Step::COMPILE) ? cfg.get_compilation_flags() : cfg.get_link_flags();
        for (const std::string& flag : flags) {
            cmd.push_back(flag);
        }

        cmd.push_back(deps[i]);
        cmd.push_back("-o");
        cmd.push_back(output[i]);

        try_compile(cmd, cfg);
    }
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Running '<MultiRule> " + name + "'", get_loc());
}

CleanRule::CleanRule(std::string name, std::vector<std::string> targets, Location _loc) try
    : Rule("Clean", std::move(name), std::move(targets), _loc) {
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Constructing '<Clean> " + name + "'", _loc);
}

/** There is no condition on cleaning */
bool CleanRule::should_run() const { return true; }

void CleanRule::run(const Config&) const try {
    char proc_name[] = "rm";
    std::vector<char*> args = {proc_name};
    args.reserve(deps.size() + 1);
    for (const std::string& d : deps) {
        // const_cast to deal with posix being posix :(
        // posix_spawnp won't accept const char*
        args.push_back(const_cast<char*>(d.data()));
    }

    pid_t pid;
    int spawn_res = posix_spawnp(&pid, proc_name, nullptr, nullptr, args.data(), environ);
    if (spawn_res != 0) {
        throw SystemError("Clean failed for '" + qualifier + " " + name + "'");
    }

    int status;
    waitpid(pid, &status, 0);
    if (status != 0) {
        throw SystemError("Clean failed for '" + qualifier + " " + name + "'");
    }
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Running '<Clean> " + name + "'", get_loc());
}