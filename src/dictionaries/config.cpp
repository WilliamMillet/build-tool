#include "config.hpp"

Config::Config(std::string _name, std::string _compiler, std::string _default_rule,
               std::vector<std::string> _compilation_flags, std::vector<std::string> _link_flags)
    : name(_name),
      compiler(_compiler),
      compilation_flags(_compilation_flags),
      link_flags(_link_flags),
      default_rule(_default_rule) {}

const std::string& Config::get_name() const { return name; }

const std::string& Config::get_compiler() const { return compiler; }

const std::vector<std::string>& Config::get_compilation_flags() const { return compilation_flags; }

const std::vector<std::string>& Config::get_link_flags() const { return link_flags; }

const std::string& Config::get_default_rule() const { return default_rule; }
