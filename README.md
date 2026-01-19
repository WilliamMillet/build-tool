# build-tool
A custom build tool for C/C++. Contains a custom configuration DSL + a lexer, parser, command runner and more. The aim of this project was to create a more modern feeling and aesthetic `make` with additional features. I am yet to come up with an interesting name for the project :(

## Language and Syntax Guide
An example configuration file, in full, may look as follows:
<!-- Note that ruby is not related to the project. It is happens to highlight the syntax well  -->
```ru
<Config> cfg {
    compiler = "clang++"
    compilation_flags = ["-g", "-Wall"]
    link_flags = [] # Example comment!
    default = "app" 
}

src_dir = "src"

cpp_files = files(src_dir)
cpp_names = file_names(cpp_files)

<Clean> clean {
    remove = ["app"] + cpp_names
}

<Rule> app {
    deps = cpp_names
    step = Step::LINK
}

<MultiRule> compilation {
    deps = cpp_files
    output = cpp_names
    step = Step::COMPILE
}
```
Build files are made up of various variables, stored at identifiers. These identifiers must contain alphanumeric characters and underscores. to assign to a variable, you can adhere to the following pattern for non-dictionary types:
```
[Identifier] = [Value][Newline]
```
Dictionaries will be explained later.

Variables are *immutable* and cannot be redefined. Additionally, they may be defined in order, as there evaluation order will be determined by the tool. However, any cyclical dependencies between variables will result in an error:
```ru
# Valid
a = b + "Bar"
b = "Foo"

# Invalid
a = b + "Bar"
b = a + "Foo
```
The following fundamental data types exist:
### String
A sequence of UTF-8 characters. One way to construct this is via a quotation enclosed string literal. Addition is defined on strings as a concatenation operation.
```ru
my_string = "Hello World"
```
### Enum
Enums in this language are similar to scoped enums in C++. You provide a scope and a name
```ru
my_enum = Step::COMPILE
```
The current enums that exist are:
- **Step**: (COMPILE, LINK)
### List
An sequence of data types. This can be homogenous or heterogeneous, although there are currently no uses for heterogeneous lists. Addition is defined on lists as a concatenation operation. Lists can be nested within each other freely:
```ru
my_list = [[[[[]]]], ["My list", []]]
```
### Dictionary
A dictionary is a map between identifiers and variables. 
```
(Optional)[Qualifier] {[Newline]
    [Identifier_1] = [Val_1]
    [Identifier_2] = [Val_2]
    ...             
    [Identifier_n] = [Val_n]
}
```
> Note that non-newline whitespace is removed during the lexing stage, so it does not matter if you prefix your identifiers with whitespace, or how much you do.

You can include as many (including 0) values as you like in the dictionary. A dictionary may be *qualified* by including an arrow enclosed qualifier
```ru
<MultiRule> compilation {
    deps = cpp_files
    output = cpp_names
    step = Step::Compile
}
```
The qualifier provides additional information about a dictionary that will determine it's role in the program. 

## Qualifiers
The following qualifiers exist in the language
- `<Config>`
- `<Rule>`
- `<MultiRule>`
- `<Clean>`

### `<Config>`
The config qualifier allows you tp specify information about your project which can be used in compilation/linking. There can only be one at anytime, so if multiple exist an error will be raised. Config Fields:
| Field | Type | Optional | Description |
| :--- | :--- | :--- | :--- |
| **compiler** | String | No | The compiler you wish to compile your project with |
| **compilation_flags** | List[String] | Yes | The flags that should be set during compilation |
| **link_flags** | List[String] | Yes | The flags that should be set during linking. |
| **default** | String | No | The default rule to run if nothing is specified |

### `<Rule>`
A Rule is a structure that represents a single build command. The name of the output will be the name of the rule. In the example case below, this would be `app`:
```ru
<Rule> app {
    deps = cpp_names
    step = Step::LINK
}
```
Rule Fields::
| Field | Type | Optional | Description |
| :--- | :--- | :--- | :--- |
| **deps** | List[String] | No | The list of dependencies that should be used when compiling the project |
| **step** | Step (Enum) | No | The build step this represents. This field is used to determine whether the compilation or linking flags should be included. |
### `<MultiRule>`
A MultiRule is similar to a regular rule in most ways, but distinct in that it represents multiple build commands, each of which having exactly one dependency. Instead of having a single output, it will have a list `output` of output values. This is in addition to the `deps` list of strings. These lists will be interpreted such that `output[i]` is the output for `deps[i]`.
MultiRule Fields:
| Field | Type | Optional | Description |
| :--- | :--- | :--- | :--- |
| **output** | List[String] | No | A list of output values where `output[i]` is the result of the build command for `deps[i]`. |
| **deps** | List[String] | No | A list of dependencies where `deps[i]` is the input for the build command producing `output[i]`. |
| **step** | Step (Enum) | No | The build step this represents. |
### `<Clean>`
A special type of rule that allows for the removal of files.
```ru
<Clean> clean {
    remove = ["a", "b", "c"]
}
```
| Field | Type | Optional | Description |
| :--- | :--- | :--- | :--- |
| **remove** | List[String] | No | A list of files to remove. Use with caution. |
## Functions
Built in functions can be called, and composed to manipulate data structures. These are, as of January 19th:
| Function name | Arguments | Returns | Description |
| Function name | Arguments | Returns | Description |
| :--- | :--- | :--- | :--- |
| **file_names** | List[String] | List[String] | Strips the file extensions off a list of file names |

## Comments
You can write comments by inverting the `#` symbol before the comment. These will be ignored by the parser once lexing is completed.
## Error System
This tool was designed to provide clear, human-readable and locatable errors upon failure. The core pieces of information provided in the case of an error are:
- A message
- An error type
- A file and location (line + column)
- An excerpt from the error location pointing to the error
- Context about the events occurring at the time of the error

An example error in this format is:
```ru
Exception thrown: SyntaxError
Message: Unexpected char '@'
Location: test_data/Buildfile:0:22:
0 |   compilation_flags =@ ["-g", "-Wall"]
  |                      ^ error here
  |    link_flags = []
  |
  |    default = "app" # This is the equivalent of 'all' from make
Context: Error occurred during:
- [Lexing]
- [Consuming character]
```
The types of errors that may occur are
### UnknownError
It is not known why the error occurred.
### IOError
Failed interaction with files.
### SyntaxError
Parsing failed as the syntax is malformed.
### TypeError
The wrong type is given (e.g. List is used over Map).
### ValueError
The correct type is given, but the value itself is wrong.
### LogicError
There is a logical issue with the program. E.g. cyclical dependencies.
### SystemError
External error related to the system itself like a process failing to spawn. The message associated with the `errno` macro value at the time of construction will be used in addition to any user provided message.
## Optimisation
The runner is designed to follow an incremental compilation model, where compilation will only be performed if changes were made to a direct or indirect dependency of a rule. This is determined by checking dependencies and recursively checking all of there dependencies, comparing file write timestamps at each step.
## Technical Design Overview
The architecture follows a processing pipeline, consisting of the following high level operations:
- Lexing
- Parsing
- Variable Evaluation
- Graph formation
- Rule running

### Lexing
The lexing process involves iterating over the build file, converted individual or groups of characters into `Lexeme` structs. This is done by a `Lexer` object which interacts with various `FileUtils` namespace functions. The result of the lexing process will be a vector of lexemes, each of which will be represented as follows:
```cpp
struct Lexeme {
    LexemeType type;
    std::string value;
    Location loc;
};
```
> Location is the position in the file, line number and column number. Purely used for error pinpointing
Certain structures such as strings and identifiers have their characters grouped into a single unit. This simplifies the parsing process

### Parsing
The parsing process, performed by the `Parser` object is responsible for converting lexemes into several Abstract Syntax Trees (AST). The first step of this is to isolate each individual variable. These can be treated as a set of distinct trees rather then a single tree or sequence, as order is irrelevant. The AST is not responsible for handling the interaction between these trees. To perform this isolation, the parsing looks for an assignment, then consumes until it determines the variable has ended. For non-dictionaries this occurs when a newline is hit. For a dictionary, it occurs when the number of closing braces found is equal to the number of opening braces. From this, a collection of `VarLexemes` structs is formed:
```cpp
struct VarLexemes {
    std::string identifier;
    std::vector<Lexeme> lexemes;
    VarCategory category;
    Location start_loc;
};
```
These `VarLexemes` structs can then be transformed into a expression trees via a the parse_expr function which begins the recursive descent parsing 
```cpp
std::unique_ptr<Expr> Parser::parse_expr()
```
This builds up a tree from the root with special rules for each sub-expression type detected (functions, dictionaries, etc).
Once the tree is formed, this expression can be used to add to a parsed variables vector which will eventually be returned. This stores the identifier, expression, qualifier if it exists, and location.
```cpp
struct VarLexemes {
    std::string identifier;
    std::vector<Lexeme> lexemes;
    VarCategory category;
    Location start_loc;
};

struct ParsedVariable {
    std::string identifier;
    std::unique_ptr<Expr> expr;
    VarCategory category;
    Location loc;
};
```
Expr itself is implemented as a virtual class, with derived classes such as `BinaryOpExpr`, `StringExpr`, `EnumExpr`, and more.
### Variable evaluation
The variable evaluation step, performed by the `VariableEvaluator` class is responsible for taking a collection of parsed variables and evaluating them. Since variables can be defined in any order, the first step of this is to determine the order to perform the evaluation. It is necessary that for any variable V, the dependencies of V are evaluated before it. The dependencies of each expression can be aggregated by performing breadth-first search on the expression, accessing the sub-expressions of a node via the `get_children` virtual method which accepts as a common interface. Once variables are aggregated, an adjacency list can be formed where `adj[v] = dep[v]`. Kahn's algorithm for topological sort allows for us to find the order we desire.

Once sorted, we can evaluate each `Expr` using the polymorphic `evaluate` method which recursively evaluates each element of the tree. From this we can form a dictionary of identifier variable mappings. It is important to note that only qualified dictionaries are relevant to the final build process, non-qualified dictionary variables only exist to be evaluated in qualified dictionaries. Therefore, we will only return the evaluated qualified dictionaries:
```cpp
struct QualifiedDicts {
    std::vector<std::unique_ptr<Rule>> rules;
    Config cfg;
};
```
The final return value of the variable evaluation step will be a configuration object and vector of rules. 

### Graph formation
A directed, acyclical graph (DAG) of rules is formed based on rules and there dependencies. This is necessary for rules that depend on other rules. An adjacency matrix is designed for this purpose as the graph my be sparse.

### Rule running
The final and most important step of the build process is to actually run the rules a user wishes to run. The `run_rule` function in the `RuleRunner` class is responsible for organising this. This performs a post-order DFS on the rule DAG. Once the call stack begins to unwind, a comparison will be made with each graph node and it's children to determine if a rebuild is necessary. If any child has a more recent file write time then the parent, the parent should be recompiled. Compiling only when a descendant node has updated facilitates significantly shorter compile times. The pseudocode for this algorithm looks as follows:
```
function run_rule(id, visited):
    # Skip files that are not recipes and visited recipes
    if is is not rule or rule is visited:
        continue
    
    for dep in dependencies[id]:
        run_rule(dep)
    
    add id to visited

    # For rules should_run is based on file times. For Clean it always returns true
    if should_run(id):
        execute(id)
```
The execution function is virtually defined on the Rule base class. For `<Rule>` and `<MultiRule>` it uses the POSIX spawn API to spawn the users desired compiler process and run there desired command in accordance with the `<Config>` they set. For `<Clean>`, it uses POSIX spawn to use run the `rm` command.

# Requirements
The tool only works on a POSIX-compliant OS with Unix utilities like `rm`

# More features coming soon :)
