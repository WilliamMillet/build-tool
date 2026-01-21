#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

class Config {
   public:
    Config(std::string _name, std::string _compiler, std::string _default_rule,
           std::vector<std::string> _compilation_flags, std::vector<std::string> _link_flags);

    const std::string& get_name() const;
    const std::string& get_compiler() const;
    const std::vector<std::string>& get_compilation_flags() const;
    const std::vector<std::string>& get_link_flags() const;
    const std::string& get_default_rule() const;

   private:
    std::string name;
    std::string compiler;
    std::vector<std::string> compilation_flags;
    std::vector<std::string> link_flags;
    std::string default_rule;
};

#endif