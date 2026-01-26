#include "utils.hpp"

#include <memory>

#include "src/dictionaries/config_factory.hpp"
#include "src/parsing/expr.hpp"

using namespace std::chrono_literals;

/**
 * Create a configuration dictionary
 * @note compiler is passed as an expr for testing non-string arguments
 */
std::unique_ptr<DictionaryExpr> Factories::create_cfg_dict(std::unique_ptr<Expr> compiler,
                                                           Flags compiler_flags, Flags link_flags,
                                                           std::string default_rule) {
    std::unique_ptr<DictionaryExpr> cfg = std::make_unique<DictionaryExpr>();

    cfg->insert_entry(ConfigFactory::COMPILER_FIELD, std::move(compiler));

    std::vector<std::unique_ptr<Expr>> c_flags;
    for (std::string flag : compiler_flags) {
        c_flags.push_back(std::make_unique<StringExpr>(flag));
    }
    cfg->insert_entry(ConfigFactory::COMPILATION_FLAGS_FIELD,
                      std::make_unique<ListExpr>(std::move(c_flags)));

    std::vector<std::unique_ptr<Expr>> l_flags;
    for (std::string flag : link_flags) {
        l_flags.push_back(std::make_unique<StringExpr>(flag));
    }

    cfg->insert_entry(ConfigFactory::LINK_FLAGS_FIELD,
                      std::make_unique<ListExpr>(std::move(l_flags)));

    cfg->insert_entry(ConfigFactory::DEFAULT_FIELD,
                      std::make_unique<StringExpr>(std::move(default_rule)));

    return cfg;
}

std::unique_ptr<DictionaryExpr> Factories::create_cfg_dict() {
    return create_cfg_dict(std::make_unique<StringExpr>("clang++"), {"-Werror", "-Wall"}, {},
                           "my_rule");
}

std::filesystem::file_time_type Time::past() {
    return std::filesystem::file_time_type::clock::now() - 10s;
};
std::filesystem::file_time_type Time::future() {
    return std::filesystem::file_time_type::clock::now() + 10s;
};

std::filesystem::path IO::get_test_file_path(std::filesystem::path file) {
    return std::filesystem::path(TEST_DATA_DIR / file);
}