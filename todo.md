- String quote escaping
- Cleaner error messages. Maybe show the actual line when throwing the error
- Look into better ways to throw exceptions without swallowing up stuff like exception type.
- Maybe I could have my own Python-like exception hierachy with name error, index error, etc
- Possibly change Value to be more polymorphic and more open closed
- Move all variables for classes to private for encapsulatory purposes
- Add @exception to the function comments maybe
- Possibly add an iterator to ValueLists
- Gotta read slightly more in depth into the whole parser thing. My current structure is shit
- This could be good? https://pgrandinetti.github.io/compilers/page/how-to-design-a-parser/
- Maybe implement a toString method for Value to simplify debugging
- Define += as well as + for values
- Consider replacing the variant system I have with polymorphsim. This may be much more idiomatic
    - If I do this I can avoid a lot of the incomplete definition issues I faced earlier like not being able to define `vectorise`
- Consider adding more templates to the Value file. E.g. something that allows you to get a type (like std::string) from an enum (like ValueType::STRING), vice versa or both.
- As of the 16th my parser does not handle lists maybe? fix that

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