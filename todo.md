- String quote escaping
- Cleaner error messages. Maybe show the actual line when throwing the error
- Look into better ways to throw exceptions without swallowing up stuff like exception type.
- Maybe I could have my own Python-like exception hierachy with name error, index error, etc
- Possibly change Value to be more polymorphic and more open closed
- Move all variables for classes to private for encapsulatory purposes
- Add @exception to the function comments maybe

- Gotta read slightly more in depth into the whole parser thing. My current structure is shit
- This could be good? https://pgrandinetti.github.io/compilers/page/how-to-design-a-parser/

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

```
- Subgoals for the `IdentifierRegistry` -> `VariableEvaluator` refactor:
    - Create the `Config` object`
    - Create the `Rule` ABC and it's implementations
    - Create a custom constructor for each rule type
    - Make the necessary modifications too `IdentifierRegistry`

Test to do 
- Value. Test for a few types:
    - move constructor
    - copy constructor
    - move assignment operator
    - copy asignment operator destructor
    - addition