#include <memory>

#include "src/parsing/expr.hpp"

using Flags = std::vector<std::string>;

namespace Testing {
namespace Factories {

/**
 * Builds the following config expression:
 * <Config> cfg {
 *     compiler = "clang++"
 *     compilation_flags = ["-g", "-Wall"]
 *     link_flags = []
 *     default = "app" # This is the equivalent of 'all' from make
 * }
 */
std::unique_ptr<DictionaryExpr> create_cfg(std::unique_ptr<Expr> compiler, Flags compiler_flags,
                                           Flags link_flags, std::string default_rule);

/** No arg overload of create_cfg for when less control is required */
std::unique_ptr<DictionaryExpr> create_cfg();

}  // namespace Factories
}  // namespace Testing