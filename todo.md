HIGH PRIORITY
- Have it so clean only works if the file dosent exist

- String quote escaping
- Move all variables for classes to private for encapsulatory purposes
- Add @exception to the function comments maybe
- Possibly add an iterator to ValueLists
- Gotta read slightly more in depth into the whole parser thing. My current structure is shit
- This could be good? https://pgrandinetti.github.io/compilers/page/how-to-design-a-parser/
- Maybe implement a toString method for Value to simplify debugging
- Implement an equivalent of make -j. Apparently topological sort is useful for this
- Right now, MultiRule compiles everything, even if only one command needs running. Maybe fix this
- Make errors colourful. Maybe use a library for this. Could be a good opportunity for this

# REALLY GOOD IDEAS
- Have a cache of the file data. If the build file didnt change between runs, I can just used the cache stuff


Dev roadmap
- Redesign `IdentifierRegistry` to become `VariableEvaluator`. It should have an evaluate_all method which returns the following struct
```cpp
struct EvaluatedVariables {
    std::vector<std::unique_ptr<Rule>> rules
    Config cfg;
};

class Config {
    std::string compiler
    std::string compilation_flags
    std::string link_flags;
    std::string default_rule;

    Config(Value cfg_val) {
        // Join flag vectors to single strings and verify 
    }
}

Plan
COME UP WITH A MORE IN DEPTH PLAN
class RuleGraph {
    public:
        RuleGraph(Vec<Rule>) rules;

        // Create a subgraph of just files
        //   - First created a graph class which maps rule names to there dependencies
        //   - Created a subgraph of rules only (this excludes files)
        //   - Use Kahns algorithm to topologically sort the array
        bool cyclical_dep_exists() 

        Vec<String> dependencies(const String& target) const;
        
        // Get the commands of a particular target
        Vec<String> get_build_cmds(const Config& cfg, const String& target) const;

        bool rule_exists(const String& recipe) const

    private:
        Map<String, Rule> name_to_rule: 
        Map<String, Vec<String>> dep_map;

}

class RuleRunner {
    public:
        RuleRunner(RuleGraph graph);
    
        void run_rule(const String& rule) const;

        bool rebuild_needed(const String rule, const deps) const;

        void build_postorder(const String& rule);

        compile(const String& rule);
        
    private:
        RuleGraph graph;
        Config 
}
 


```
- Subgoals for the `IdentifierRegistry` -> `VariableEvaluator` refactor:
    - Create the `Config` object`
    - Create the `Rule` ABC and it's implementations
    - Create a custom constructor for each rule type
    - Make the necessary modifications too `IdentifierRegistry`

More Long term goals
Add better error messages like this.
```bash
TypeError: Expected "x" but got "y"
  --> BuildFile.bld:12:18
   |
12 |   my_rule { B: y }
   |                  ^
   |
note: Found while parsing '<SingleRule> my_rule'
```
To do this, I must store the characters as well as line numbers. I also want to store
line numbers in other areas of code
Test to do 
- Value. Test for a few types:
    - move constructor
    - copy constructor
    - move assignment operator
    - copy asignment operator destructor
    - addition