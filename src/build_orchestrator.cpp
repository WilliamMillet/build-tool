#include "build_orchestrator.hpp"

#include <iostream>
#include <memory>

#include "built_in/func_registry.hpp"
#include "dictionaries/qualified_dicts.hpp"
#include "errors/error.hpp"
#include "io/fs_gateway.hpp"
#include "io/proc_spawner.hpp"
#include "lexer.hpp"
#include "parsing/parser.hpp"
#include "rule_graph.hpp"
#include "rule_runner.hpp"
#include "variable_evaluator.hpp"

BuildOrchestrator::BuildOrchestrator(std::shared_ptr<FSGateway> fs,
                                     std::shared_ptr<ProcessSpawner> spawner,
                                     std::string src_file) try {
    src_filename = src_file;

    Lexer lexer{src_file};
    std::vector<Lexeme> lexed = lexer.lex();

    Parser parser(lexed);
    std::vector<ParsedVariable> parsed = parser.parse();

    FuncRegistry fn_reg;
    VariableEvaluator evaluator(std::move(parsed), fn_reg);
    QualifiedDicts qualifiers = evaluator.evaluate();

    std::shared_ptr<RuleGraph> graph = std::make_shared<RuleGraph>(std::move(qualifiers.rules));
    runner =
        std::make_unique<RuleRunner>(graph, std::make_shared<Config>(qualifiers.cfg), spawner, fs);
} catch (const Error& err) {
    std::cerr << err.format(src_file) << std::endl;
} catch (const std::exception& err) {
    std::cerr << "Failed to parse '" << src_file << "' Error: " << err.what() << std::endl;
}

void BuildOrchestrator::run_rule(std::string cmd) const try {
    runner->run_rule(cmd);
} catch (const Error& err) {
    std::cerr << err.format(src_filename) << std::endl;
} catch (const std::exception& err) {
    std::cerr << "Failed to parse '" << src_filename << "' Error: " << err.what() << std::endl;
}