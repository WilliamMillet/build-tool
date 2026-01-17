#ifndef QUALIFIED_DICTS_H
#define QUALIFIED_DICTS_H

#include "config.hpp"
#include "rules.hpp"
#include "vector"

struct QualifiedDicts {
    std::vector<std::unique_ptr<Rule>> rules;
    Config cfg;
};

#endif