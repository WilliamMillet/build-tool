#include "rules.hpp"

#include <unistd.h>

#include <cstdlib>
#include <system_error>

#include "config.hpp"
#include "spawn.h"
#include "sys/wait.h"
#include "unistd.h"

void Rule::try_compile(std::vector<std::string> cmd, Config& cfg) const {
    std::vector<char*> raw_args;
    raw_args.reserve(cmd.size());
    for (std::string& s : cmd) {
        raw_args.push_back(s.data());
    }

    pid_t proc;
    posix_spawnp(&proc, cfg.get_compiler().data(), nullptr, nullptr, raw_args.data(), environ);

    int status;
    waitpid(proc, &status, 0);
    if (status != 0) {
        throw std::system_error(
            errno, std::generic_category(),
            "Compilation failed for '<" + get_rule_type_str() + "> " + name + "'");
    }
}

SingleRule::SingleRule(std::string _name, Value obj) {
    name = std::move(_name);
    obj.assert_type(ValueType::Dictionary);
    Dictionary dict = obj.get<Dictionary>();
    dict.assert_contains(
        {{RuleFields::NAME, ValueType::STRING}, {RuleFields::STEP, ValueType::ENUM}});

    deps = ValueUtils::vectorise<std::string>(dict.get(RuleFields::NAME).get<ValueList>());
    step = resolve_enum<Step>(dict.get(RuleFields::STEP).get<ScopedEnumValue>());
}

MultiRule::MultiRule(std::string _name, Value obj) {
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

CleanRule::CleanRule(std::string _name, Value obj) {
    name = std::move(_name);
    obj.assert_type(ValueType::Dictionary);
    Dictionary dict = obj.get<Dictionary>();
    dict.assert_contains({{RuleFields::TARGETS, ValueType::LIST}});
    deps = ValueUtils::vectorise<std::string>(dict.get(RuleFields::NAME).get<ValueList>());
}

const std::string& MultiRule::get_rule_type_str() const { return RULE_TYPE_STR; }

const std::string& SingleRule::get_rule_type_str() const { return RULE_TYPE_STR; }

const std::string& CleanRule::get_rule_type_str() const { return RULE_TYPE_STR; }

void SingleRule::run(Config& cfg) const {
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

void MultiRule::run(Config& cfg) const {
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

void CleanRule::run(Config&) const {
    char proc_name[] = "rm";
    std::vector<char*> args = {proc_name};
    args.reserve(deps.size() + 1);
    for (const std::string& d : deps) {
        // const_cast to deal with posix being posix :(
        // posix_spawnp won't accept const char*
        args.push_back(const_cast<char*>(d.data()));
    }

    pid_t pid;
    posix_spawnp(&pid, proc_name, nullptr, nullptr, args.data(), environ);
}
