#include <iostream>
#include <stdexcept>

#include "built_in/func_registry.hpp"
#include "dictionaries/qualified_dicts.hpp"
#include "errors/error.hpp"
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
        RuleGraph graph(std::move(qualifiers.rules));
        RuleRunner runner(std::move(graph), std::move(qualifiers.cfg));
    } catch (const Error& err) {
        std::cerr << err.format(src) << std::endl;
    } catch (const std::exception& err) {
        std::cerr << "Failed to parse '" << src << "' Error: " << err.what() << std::endl;
    }
}