#ifndef CONFIG_H
#define CONFIG_H

#include <string>

#include "../value.hpp"

class Config {
   public:
    Config(std::string _name, Value dict);

    const std::string& get_compiler() const;
    const std::string& get_compilation_flags() const;
    const std::string& get_link_flags() const;
    const std::string& get_default_rule() const;

   private:
    inline const static std::string COMPILER_FIELD = "compiler";
    inline const static std::string COMPILATION_FLAGS_FIELD = "compilation_flags";
    inline const static std::string LINK_FLAGS_FIELD = "link_flags";
    inline const static std::string DEFAULT_SINGLE_RULE_FIELD = "default_rule";

    std::string name;
    std::string compiler;
    std::string compilation_flags;
    std::string link_flags;
    std::string default_rule;

    /** Join a ValueList of strings into a space separated string and return this */
    std::string join_list(ValueList list) const;
};

#endif