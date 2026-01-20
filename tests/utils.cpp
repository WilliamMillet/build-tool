#include "utils.hpp"

#include <memory>

#include "src/dictionaries/config.hpp"
#include "src/parsing/expr.hpp"

std::unique_ptr<DictionaryExpr> TestUtils::ExprFactories::create_cfg(std::string compiler,
                                                                     Flags compiler_flags,
                                                                     Flags link_flags,
                                                                     std::string default_rule) {
    std::unique_ptr<DictionaryExpr> cfg = std::make_unique<DictionaryExpr>();
    auto& cmap = cfg->fields_map;

    cmap[Config::COMPILER_FIELD] = std::make_unique<StringExpr>(std::move(compiler));

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