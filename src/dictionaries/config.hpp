#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

struct Config {
    std::string name;
    std::string compiler;
    std::vector<std::string> compilation_flags;
    std::vector<std::string> link_flags;
    std::string default_rule;
};

#endif