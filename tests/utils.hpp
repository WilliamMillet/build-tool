#include <memory>
#include <unordered_map>

#include "src/parsing/expr.hpp"
#include "src/rule_graph.hpp"

namespace Factories {
using Flags = std::vector<std::string>;

/**
 * Builds the following config expression:
 * <Config> cfg {
 *     compiler = "clang++"
 *     compilation_flags = ["-g", "-Wall"]
 *     link_flags = []
 *     default = "app" # This is the equivalent of 'all' from make
 * }
 */
std::unique_ptr<DictionaryExpr> create_cfg_dict(std::unique_ptr<Expr> compiler,
                                                Flags compiler_flags, Flags link_flags,
                                                std::string default_rule);

/** No arg overload of create_cfg_dict for when less control is required */
std::unique_ptr<DictionaryExpr> create_cfg_dict();

/**
 * Create a rule graph based on an adjacency list
 * @tparam The type of rules in the graph
 * @param adj_list An adjacency list of rules and dependencies
 */
template <typename T>
RuleGraph create_graph(std::unordered_map<std::string, std::string> adj_list) {
    std::vector<std::unique_ptr<Rule>> rules;
    for (const auto& [v, adj] : adj_list) {
        rules.push_back(std::make_unique<T>(v, adj, Step::COMPILE, Location{}));
    }

    return rules;
};
}  // namespace Factories

namespace Time {
std::filesystem::file_time_type past();
std::filesystem::file_time_type future();
}  // namespace Time

namespace IO {
inline const std::string TEST_DATA_DIR = "../tests/data/files";

/** Get the path of a file in the file directory of test data */
std::filesystem::path get_test_file_path(std::filesystem::path file);
}  // namespace IO