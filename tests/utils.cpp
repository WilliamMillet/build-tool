#include "utils.hpp"

#include <memory>

#include "src/dictionaries/config.hpp"
#include "src/parsing/expr.hpp"

// compiler is passed as an expr for testing non-string arguments
std::unique_ptr<DictionaryExpr> Testing::Factories::create_cfg(std::unique_ptr<Expr> compiler,
                                                               Flags compiler_flags,
                                                               Flags link_flags,
                                                               std::string default_rule) {
    std::unique_ptr<DictionaryExpr> cfg = std::make_unique<DictionaryExpr>();
    auto& cmap = cfg->fields_map;

    cmap[Config::COMPILER_FIELD] = std::move(compiler);

    std::vector<std::unique_ptr<Expr>> c_flags;
    for (std::string flag : compiler_flags) {
        c_flags.push_back(std::make_unique<StringExpr>(flag));
    }
    cmap[Config::COMPILATION_FLAGS_FIELD] = std::make_unique<ListExpr>(std::move(c_flags));

    std::vector<std::unique_ptr<Expr>> l_flags;
    for (std::string flag : link_flags) {
        l_flags.push_back(std::make_unique<StringExpr>(flag));
    }

    cmap[Config::LINK_FLAGS_FIELD] = std::make_unique<ListExpr>(std::move(l_flags));

    cmap[Config::DEFAULT_FIELD] = std::make_unique<StringExpr>(std::move(default_rule));

    return cfg;
}

std::unique_ptr<DictionaryExpr> Testing::Factories::create_cfg() {
    return create_cfg(std::make_unique<StringExpr>("clang++"), {"-Werror", "-Wall"}, {}, "my_rule");
}
