#include "rules.hpp"

#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>

#include "config.hpp"
#include "spawn.h"
#include "sys/wait.h"
#include "unistd.h"

namespace fs = std::filesystem;

Rule::Rule(std::string qualifier_str) : qualifier("<" + qualifier_str + ">") {};

void Rule::try_compile(std::vector<std::string>& cmd, const Config& cfg) const {
    std::vector<char*> raw_args;
    raw_args.reserve(cmd.size());
    for (std::string& s : cmd) {
        raw_args.push_back(s.data());
    }

    const char* compiler_arr = cfg.get_compiler().data();
    pid_t proc;
    int spawn_res = posix_spawnp(&proc, compiler_arr, nullptr, nullptr, raw_args.data(), environ);
    if (spawn_res != 0) {
        throw std::system_error(errno, std::generic_category(),
                                "Compilation failed for '" + qualifier + " " + name + "'");
    }

    int status;
    waitpid(proc, &status, 0);
    if (status != 0) {
        throw std::system_error(errno, std::generic_category(),
                                "Compilation failed for '" + qualifier + " " + name + "'");
    }
}

bool Rule::has_updated_dep() const {
    if (!fs::exists(name)) return true;

    auto target_write_t = fs::last_write_time(name);
    return std::ranges::any_of(deps, [&](std::string d) {
        return !fs::exists(d) || fs::last_write_time(d) >= target_write_t;
    });
}

SingleRule::SingleRule(std::string _name, Value obj) : Rule("Rule") {
    name = std::move(_name);
    obj.assert_type(ValueType::Dictionary);
    Dictionary dict = obj.get<Dictionary>();
    dict.assert_contains(
        {{RuleFields::NAME, ValueType::STRING}, {RuleFields::STEP, ValueType::ENUM}});

    deps = ValueUtils::vectorise<std::string>(dict.get(RuleFields::NAME).get<ValueList>());
    step = resolve_enum<Step>(dict.get(RuleFields::STEP).get<ScopedEnumValue>());
}

bool SingleRule::should_run() const { return has_updated_dep(); }

void SingleRule::run(const Config& cfg) const {
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
}

MultiRule::MultiRule(std::string _name, Value obj) : Rule("MultiRule") {
    name = std::move(_name);
    obj.assert_type(ValueType::Dictionary);
    Dictionary dict = obj.get<Dictionary>();
    dict.assert_contains({{"deps", ValueType::LIST},
                          {RuleFields::OUTPUT, ValueType::LIST},
                          {RuleFields::STEP, ValueType::ENUM}});

    deps = ValueUtils::vectorise<std::string>(dict.get(RuleFields::NAME).get<ValueList>());
    output = ValueUtils::vectorise<std::string>(dict.get(RuleFields::OUTPUT).get<ValueList>());

    if (deps.size() != output.size()) {
        throw std::invalid_argument(
            "Error in MultiRule '" + name + "'. 'deps' length (" + std::to_string(deps.size()) +
            ") is not the same as 'output' length (" + std::to_string(output.size()) + ").");
    }
    step = resolve_enum<Step>(dict.get(RuleFields::STEP).get<ScopedEnumValue>());
}

bool MultiRule::should_run() const { return has_updated_dep(); }

void MultiRule::run(const Config& cfg) const {
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
}

CleanRule::CleanRule(std::string _name, Value obj) : Rule("Clean") {
    name = std::move(_name);
    obj.assert_type(ValueType::Dictionary);
    Dictionary dict = obj.get<Dictionary>();
    dict.assert_contains({{RuleFields::TARGETS, ValueType::LIST}});
    deps = ValueUtils::vectorise<std::string>(dict.get(RuleFields::NAME).get<ValueList>());
}

/** There is no condition on cleaning */
bool CleanRule::should_run() const { return true; }

void CleanRule::run(const Config&) const {
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
        throw std::system_error(errno, std::generic_category(),
                                "Clean failed for '" + qualifier + " " + name + "'");
    }

    int status;
    waitpid(pid, &status, 0);
    if (status != 0) {
        throw std::system_error(errno, std::generic_category(),
                                "Clean failed for '" + qualifier + " " + name + "'");
    }
}