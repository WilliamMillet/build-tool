#ifndef VALUE_REGISTRY_H
#define VALUE_REGISTRY_H

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

#include "lexer.hpp"
#include "value.hpp"

struct UnevaluatedVariable {
    std::string name;
    std::vector<std::string> deps;
    std::vector<Lexeme> lexemes;
};

class VariableRegistry {
   public:
    VariableRegistry(std::vector<Lexeme> lexemes);

   private:
    std::unordered_map<std::string, Value> variables;
    std::vector<Lexeme> lexemes;
    size_t parse_pos = 0;

    std::array<LexemeType, 2> valid_identifier_successors = {LexemeType::EQUALS,
                                                             LexemeType::BLOCK_START};

    std::array<LexemeType, 6> variable_starts = {LexemeType::IDENTIFIER, LexemeType::ADD,
                                                 LexemeType::LIST_START, LexemeType::MACRO_FN_START,
                                                 LexemeType::STRING};

    /** Get the lexeme at the current position */
    Lexeme peek() const;

    /** Advance the position in the lexemes and return what was 'consumed' */
    Lexeme consume();

    /** True iff the position is after all lexemes */
    bool at_end() const;

    /** Parse what is potentially an assignment, knowing it starts with an identifier */
    std::unique_ptr<UnevaluatedVariable> parse_potential_assignment();

    /**
     * Consume a lexeme expression that is being assigned to a val and return it in unevaluated form
     * @param identifier The identifier the lexeme expression is being assigned to
     */
    UnevaluatedVariable consume_expr(std::string assigning_to);

    /** Parse the value of a variable */
    Value parse_value();

    void throw_pinpointed_err(std::string msg) const;
};

#endif