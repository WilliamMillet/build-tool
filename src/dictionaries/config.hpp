#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

#include "../value.hpp"

class Config {
   public:
    Config(std::string _name, Value dict);

    const std::string& get_compiler() const;
    const std::vector<std::string>& get_compilation_flags() const;
    const std::vector<std::string>& get_link_flags() const;
    const std::string& get_default_rule() const;

   private:
    inline const static std::string COMPILER_FIELD = "compiler";
    inline const static std::string COMPILATION_FLAGS_FIELD = "compilation_flags";
    inline const static std::string LINK_FLAGS_FIELD = "link_flags";
    inline const static std::string DEFAULT_SINGLE_RULE_FIELD = "default_rule";

    std::string name;
    std::string compiler;
    std::vector<std::string> compilation_flags;
    std::vector<std::string> link_flags;
    std::string default_rule;
};

#endif