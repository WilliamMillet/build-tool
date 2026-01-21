
#include "../value.hpp"
#include "config.hpp"

class ConfigFactory {
   public:
    inline const static std::string COMPILER_FIELD = "compiler";
    inline const static std::string COMPILATION_FLAGS_FIELD = "compilation_flags";
    inline const static std::string LINK_FLAGS_FIELD = "link_flags";
    inline const static std::string DEFAULT_FIELD = "default_rule";

    /** Create a config based on a dictionary value object */
    Config make_config(std::string id, Value cfg_val) const;
};