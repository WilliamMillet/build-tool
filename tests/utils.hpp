#include <memory>

#include "src/parsing/expr.hpp"

using Flags = std::vector<std::string>;

namespace TestUtils {
namespace ExprFactories {

/**
 * Builds the following config expression:
 * <Config> cfg {
 *     compiler = "clang++"
 *     compilation_flags = ["-g", "-Wall"]
 *     link_flags = []
 *     default = "app" # This is the equivalent of 'all' from make
 * }
 */
std::unique_ptr<DictionaryExpr> create_cfg(std::string compiler, Flags compiler_flags,
                                           Flags link_flags, std::string default_rule);

}  // namespace ExprFactories
}  // namespace TestUtils