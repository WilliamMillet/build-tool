#include "rules.hpp"

#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <ranges>
#include <string>

#include "../errors/error.hpp"
#include "config.hpp"

Rule::Rule(std::string _qualifier, std::string _name, std::vector<std::string> _deps, Location _loc)
    : qualifier("<" + _qualifier + ">"), name(_name), deps(_deps), loc(_loc) {};

const std::vector<std::string>& Rule::get_deps() const { return deps; };
const std::string& Rule::get_name() const { return name; };
const Location& Rule::get_loc() const { return loc; };

bool Rule::has_updated_dep(FSGateway& fs) const {
    if (!fs.exists(name)) return true;

    auto target_write_t = fs.last_write_time(name);
    return std::ranges::any_of(deps, [&](std::string d) {
        return !fs.exists(d) || fs.last_write_time(d) > target_write_t;
    });
}

SingleRule::SingleRule(std::string _name, std::vector<std::string> deps, Step _step,
                       Location _loc) try
    : Rule("Rule", std::move(_name), std::move(deps), _loc), step(_step) {
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Constructing '<Rule> " + _name + "'", _loc);
}

std::vector<Command> SingleRule::get_commands(const Config& cfg) const try {
    std::vector<std::string> cmd = {cfg.compiler};

    auto& flags = (step == Step::COMPILE) ? cfg.compilation_flags : cfg.link_flags;
    for (const std::string& flag : flags) {
        cmd.push_back(flag);
    }

    for (const std::string& dep : deps) {
        cmd.push_back(dep);
    }

    cmd.push_back("-o");
    cmd.push_back(name);

    return {cmd};
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Building command for '<Rule> " + name + "'", loc);
}

bool SingleRule::should_run(FSGateway& fs) const { return has_updated_dep(fs); }

MultiRule::MultiRule(std::string _name, std::vector<std::string> _deps,
                     std::vector<std::string> _out, Step _step, Location _loc) try
    : Rule("MultiRule", std::move(_name), std::move(_deps), _loc), output(_out), step(_step) {
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Constructing '<MultiRule> " + _name + "'", _loc);
}

bool MultiRule::should_run(FSGateway& fs) const { return has_updated_dep(fs); }

std::vector<Command> MultiRule::get_commands(const Config& cfg) const try {
    std::vector<Command> cmds;
    for (size_t i = 0; i < deps.size(); i++) {
        std::vector<std::string> cmd = {cfg.compiler};
        auto& flags = (step == Step::COMPILE) ? cfg.compilation_flags : cfg.link_flags;
        for (const std::string& flag : flags) {
            cmd.push_back(flag);
        }

        cmd.push_back(deps[i]);
        cmd.push_back("-o");
        cmd.push_back(output[i]);

        cmds.push_back(cmd);
    }

    return cmds;
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Building command for '<MultiRule> " + name + "'", loc);
}

std::vector<SingleRule> MultiRule::partition() const {
    std::vector<SingleRule> parts;
    for (const auto& [dep, out] : std::ranges::views::zip(deps, output)) {
        parts.push_back(SingleRule(out, {dep}, step, loc));
    }
    return parts;
};

CleanRule::CleanRule(std::string name, std::vector<std::string> targets, Location _loc) try
    : Rule("Clean", std::move(name), std::move(targets), _loc) {
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Constructing '<Clean> " + name + "'", _loc);
}

std::vector<Command> CleanRule::get_commands(const Config&) const try {
    std::vector<std::string> clean_cmd = {"rm"};
    for (const auto& d : deps) {
        clean_cmd.push_back(d);
    }

    return {clean_cmd};
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Building command for '<Clean> " + name + "'", loc);
}

/** There is no condition on cleaning */
bool CleanRule::should_run(FSGateway&) const { return true; }