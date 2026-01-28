#ifndef CONFIG_FACTORY_H
#define CONFIG_FACTORY_H

#include "../value.hpp"
#include "config.hpp"

class ConfigFactory {
   public:
    inline const static std::string COMPILER_FIELD = "compiler";
    inline const static std::string COMPILATION_FLAGS_FIELD = "compilation_flags";
    inline const static std::string LINK_FLAGS_FIELD = "link_flags";
    inline const static std::string DEFAULT_FIELD = "default_rule";

    /**
     * Create a configuration object using a parsed and evaluated value
     * @param id The identifier the configuration is assigned to
     * @param cfg_val The parsed and evaluated configuration dictionary with all required fields
     */
    Config make_config(std::string id, Value cfg_val) const;
};

#endif