#include <iostream>
#include <memory>
#include <stdexcept>

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

int main(int argc, char** argv) {
    if (argc > 2) {
        std::string use_one = "<" + std::string(argv[0]) + "> <file>";
        throw std::invalid_argument("Invalid CLI arguments. Usage:\n <" + use_one);
    }

    const std::string src = argv[1];
    try {
        Lexer lexer{src};
        std::vector<Lexeme> lexed = lexer.lex();

        Parser parser(lexed);
        std::vector<ParsedVariable> parsed = parser.parse();

        FuncRegistry fn_reg;
        VariableEvaluator evaluator(std::move(parsed), fn_reg);
        QualifiedDicts qualifiers = evaluator.evaluate();

        std::shared_ptr<RuleGraph> graph = std::make_shared<RuleGraph>(std::move(qualifiers.rules));
        std::shared_ptr<ProcessSpawner> spawner = std::make_unique<PosixProcSpawner>();
        std::shared_ptr<FSGateway> fs = std::make_unique<ProdFSGateway>();
        RuleRunner runner(graph, std::make_shared<Config>(qualifiers.cfg), std::move(spawner),
                          std::move(fs));
    } catch (const Error& err) {
        std::cerr << err.format(src) << std::endl;
    } catch (const std::exception& err) {
        std::cerr << "Failed to parse '" << src << "' Error: " << err.what() << std::endl;
    }
}