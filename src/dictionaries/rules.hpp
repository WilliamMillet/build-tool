#ifndef SINGLE_RULES_H
#define SINGLE_RULES_H

#include <string>
#include <vector>

#include "../built_in/enums.hpp"
#include "../value.hpp"
#include "config.hpp"

// TODO: This and the associated .cpp file could use significant refactors.
//  - Once I have better error handling which shows the snippet, I won't need the string constructor
// his would clean my code up
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
    Rule(std::string qualifier_str);

    const std::string& get_name() const;
    const std::vector<std::string>& get_deps() const;

    virtual ~Rule() = 0;

    /** Execute the rule (e.g. build the file if its a SingleRule, clean if its a CleanRule) */
    virtual void run(const Config& cfg) const;

    /**
     * Determine if it's necessary to run the rule at a given time (e.g. for a SingleRule, this
     * would be true if and only if any dependency was updated)
     * @note It is required 'should_run' is called in a postorder context for it's return value
     * to be accurate. This means if rule X is dependant on rule Y, which is dependant on file Z,
     * then should_run must be called on rule Y then rule X. The function is designed this way as
     * if it could be called from any position, then it would have to do it's own recursive check
     * which would lead to O(n^2) runtime for the entire traversal.
     */
    virtual bool should_run() const;

   protected:
    std::string name;
    std::vector<std::string> deps;

    /** Attempt to run a compilation command */
    void try_compile(std::vector<std::string>& cmd, const Config& cfg) const;

    /**
     * Returns true if and only if an immediate dependency's file output is newer then the 'name'
     * file
     */
    bool has_updated_dep() const;

    std::string qualifier;
};

class SingleRule : public Rule {
   public:
    SingleRule(std::string _name, Value obj);

    void run(const Config& cfg) const override;

    bool should_run() const override;

   protected:
    Step step;
};

class MultiRule : public Rule {
   public:
    MultiRule(std::string _name, Value obj);

    const std::vector<std::string>& get_output() const;

    void run(const Config& cfg) const override;

    bool should_run() const override;

   protected:
    // The output files. For all i, output[i] will be the output file for deps[i]
    std::vector<std::string> output;
    Step step;
};

class CleanRule : public Rule {
   public:
    CleanRule(std::string _name, Value obj);

    void run(const Config& cfg) const override;

    bool should_run() const override;
};

#endif