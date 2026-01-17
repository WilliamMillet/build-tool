#ifndef SINGLE_RULES_H
#define SINGLE_RULES_H

#include <string>
#include <vector>

#include "../built_in/enums.hpp"
#include "../value.hpp"

enum class RuleType { SINGLE, MULTI, CLEAN };

namespace RuleFields {
inline constexpr static std::string NAME = "name";
inline constexpr static std::string STEP = "step";
inline constexpr static std::string OUTPUT = "output";
inline constexpr static std::string TARGETS = "targets";
}  // namespace RuleFields

struct Rule {
    std::string name;
    std::vector<std::string> deps;
};

struct SingleRule : Rule {
    Step step;

    SingleRule(std::string _name, Value obj);
};

struct MultiRule : Rule {
    // The output files. For all i, output[i] will be the output file for deps[i]
    std::vector<std::string> output;
    Step step;

    MultiRule(std::string _name, Value obj);
};

struct CleanRule : Rule {
    CleanRule(std::string _name, Value obj);
};

#endif