#ifndef RULES_H
#define RULES_H

#include <string>
#include <vector>

#include "../built_in/enums.hpp"
#include "../errors/error.hpp"
#include "../io/fs_gateway.hpp"
#include "../io/proc_runner.hpp"
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
inline constexpr static std::string DEPS = "deps";
inline constexpr static std::string OUTPUT = "output";
inline constexpr static std::string TARGETS = "targets";
}  // namespace RuleFields

class Rule {
   public:
    Rule(std::string _qualifier, std::string _name, std::vector<std::string>, Location _loc);

    const std::string& get_name() const;
    const std::vector<std::string>& get_deps() const;
    const Location& get_loc() const;

    virtual ~Rule() = default;

    /** Execute the rule (e.g. build the file if its a SingleRule, clean if its a CleanRule) */
    virtual void run(const Config& cfg, ProcessRunner* process_runner) const = 0;

    /**
     * Determine if it's necessary to run the rule at a given time (e.g. for a SingleRule, this
     * would be true if and only if any dependency was updated)
     * @note It is required 'should_run' is called in a postorder context for it's return value
     * to be accurate. This means if rule X is dependant on rule Y, which is dependant on file Z,
     * then should_run must be called on rule Y then rule X. The function is designed this way as
     * if it could be called from any position, then it would have to do it's own recursive check
     * which would lead to O(n^2) runtime for the entire traversal.
     */
    virtual bool should_run(FSGateway fs) const = 0;

   protected:
    std::string qualifier;
    std::string name;
    std::vector<std::string> deps;
    Location loc;

    /**
     * Returns true if and only if an immediate dependency's file output is newer then the 'name'
     * file
     */
    bool has_updated_dep(FSGateway fs) const;
};

class SingleRule : public Rule {
   public:
    SingleRule(std::string _name, std::vector<std::string> _deps, Step step, Location _loc);

    void run(const Config& cfg, ProcessRunner* process_runner) const override;

    bool should_run(FSGateway fs) const override;

   protected:
    Step step;
};

class MultiRule : public Rule {
   public:
    MultiRule(std::string _name, std::vector<std::string> deps, std::vector<std::string> out,
              Step step, Location _loc);

    void run(const Config& cfg, ProcessRunner* process_runner) const override;

    bool should_run(FSGateway fs) const override;

   protected:
    // The output files. For all i, output[i] will be the output file for deps[i]
    std::vector<std::string> output;
    Step step;
};

class CleanRule : public Rule {
   public:
    CleanRule(std::string name, std::vector<std::string> targets, Location _loc);

    void run(const Config& cfg, ProcessRunner* process_runner) const override;

    bool should_run(FSGateway fs) const override;
};

#endif