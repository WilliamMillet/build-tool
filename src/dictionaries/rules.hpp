#ifndef SINGLE_RULES_H
#define SINGLE_RULES_H

#include <string>
#include <vector>

#include "../built_in/enums.hpp"
#include "../value.hpp"
#include "config.hpp"

// TODO: This and the associated .cpp file could use significant refactors.
//  - Once I have better error handling which shows the snippet, I won't need the get_rule_type_str
// here or the private section with the RULE_TYPE_STR. This would clean my code up
//  - Once I improve the Value interface I can simplify the constructors for this significantly
//  - Revaluate which getters need to be exposed here

enum class RuleType { SINGLE, MULTI, CLEAN };

namespace RuleFields {
inline constexpr static std::string NAME = "name";
inline constexpr static std::string STEP = "step";
inline constexpr static std::string OUTPUT = "output";
inline constexpr static std::string TARGETS = "targets";
}  // namespace RuleFields

class Rule {
   public:
    const std::string& get_name() const;
    const std::vector<std::string>& get_deps() const;

    virtual ~Rule() = 0;

    /** Execute the rule (e.g. build the file if its a SingleRule, clean if its a CleanRule) */
    virtual void run(Config& cfg) const;

    virtual const std::string& get_rule_type_str() const;

   protected:
    std::string name;
    std::vector<std::string> deps;

    /** Attempt to run a compilation command */
    void try_compile(std::vector<std::string> cmd, Config& cfg) const;
};

class SingleRule : public Rule {
   public:
    SingleRule(std::string _name, Value obj);

    void run(Config& cfg) const override;

    const std::string& get_rule_type_str() const override;

   protected:
    Step step;

   private:
    inline constexpr static std::string RULE_TYPE_STR = "Rule";
};

class MultiRule : public Rule {
   public:
    MultiRule(std::string _name, Value obj);

    const std::vector<std::string>& get_output() const;

    void run(Config& cfg) const override;

    const std::string& get_rule_type_str() const override;

   protected:
    // The output files. For all i, output[i] will be the output file for deps[i]
    std::vector<std::string> output;
    Step step;

   private:
    inline constexpr static std::string RULE_TYPE_STR = "MultiRule";
};

class CleanRule : public Rule {
   public:
    CleanRule(std::string _name, Value obj);

    void run(Config& cfg) const override;

    const std::string& get_rule_type_str() const override;

   private:
    inline constexpr static std::string RULE_TYPE_STR = "Clean";
};

#endif