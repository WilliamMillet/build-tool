#include <string>
#include <vector>

enum class Step { COMPILE, LINK };

struct Rule {
    std::string name;
    std::vector<std::string> dependencies;
    Step step;
};

struct SingleRule : public Rule {
    std::string output;
};

struct MultiRule : public Rule {
    std::vector<std::string> ouput;
};

struct Clean {
    std::vector<std::string> targets;
};
