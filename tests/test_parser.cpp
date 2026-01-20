#include "../catch.hpp"
#include "data/parsing_data.hpp"
#include "src/lexer.hpp"
#include "src/parsing/expr.hpp"
#include "src/parsing/parser.hpp"

TEST_CASE("Test parser with empty file", "[parser]") {
    std::vector<Lexeme> lexemes = CaseLexemes::EMPTY_FILE;

    Parser parser(lexemes);
    std::vector<ParsedVariable> parsed = parser.parse();
    REQUIRE(parsed.size() == 0);
}

// This test is in one rather then split into multiple to make sure that the
// parser works with multiple variables
TEST_CASE("Test parser with simple variables", "[parser]") {
    Parser parser(CaseLexemes::SIMPLE_VARS);
    std::vector<ParsedVariable> parsed = parser.parse();

    REQUIRE(parsed.size() == 3);

    ParsedVariable& str_var = parsed.at(0);
    REQUIRE(str_var.identifier == "word");
    REQUIRE(str_var.category == VarCategory::REGULAR);
    StringExpr* str_expr = dynamic_cast<StringExpr*>(str_var.expr.get());
    REQUIRE(str_expr != nullptr);
    REQUIRE(str_expr->get_children().size() == 0);
    REQUIRE(str_expr->val == "Apple");
    REQUIRE(str_var.loc == Location{0, 0, 0});

    ParsedVariable& enum_var = parsed.at(1);
    REQUIRE(enum_var.identifier == "my_enum");
    REQUIRE(enum_var.category == VarCategory::REGULAR);
    EnumExpr* enum_expr = dynamic_cast<EnumExpr*>(enum_var.expr.get());
    REQUIRE(enum_expr != nullptr);
    REQUIRE(enum_expr->get_children().size() == 0);
    REQUIRE(enum_expr->scope == "Step");
    REQUIRE(enum_expr->name == "LINK");
    REQUIRE(enum_var.loc == Location{0, 4, 0});

    ParsedVariable& list_var = parsed.at(2);
    REQUIRE(list_var.identifier == "my_list");
    REQUIRE(list_var.category == VarCategory::REGULAR);
    ListExpr* list_expr = dynamic_cast<ListExpr*>(list_var.expr.get());
    REQUIRE(list_expr != nullptr);
    REQUIRE(list_expr->get_children().size() == 2);

    StringExpr* first_item = dynamic_cast<StringExpr*>(list_expr->get_children().at(0));
    REQUIRE(first_item != nullptr);
    REQUIRE(first_item->val == "Dog");

    StringExpr* second_item = dynamic_cast<StringExpr*>(list_expr->get_children().at(1));
    REQUIRE(second_item != nullptr);
    REQUIRE(second_item->val == "Cat");

    REQUIRE(list_var.loc == Location{0, 10, 0});
}

TEST_CASE("Test parser with regular dictionary", "[parser]") {
    Parser parser(CaseLexemes::REGULAR_DICT);
    std::vector<ParsedVariable> parsed = parser.parse();

    REQUIRE(parsed.size() == 1);

    ParsedVariable& dict_var = parsed.at(0);
    REQUIRE(dict_var.identifier == "my_dict");
    REQUIRE(dict_var.category == VarCategory::REGULAR);
    DictionaryExpr* dict_expr = dynamic_cast<DictionaryExpr*>(dict_var.expr.get());
    REQUIRE(dict_expr != nullptr);

    REQUIRE(dict_expr->fields_map.size() == 2);
    REQUIRE(dict_expr->fields_map.count("name") == 1);
    REQUIRE(dict_expr->fields_map.count("count") == 1);

    StringExpr* name_expr = dynamic_cast<StringExpr*>(dict_expr->fields_map.at("name").get());
    REQUIRE(name_expr != nullptr);
    REQUIRE(name_expr->val == "value");

    StringExpr* count_expr = dynamic_cast<StringExpr*>(dict_expr->fields_map.at("count").get());
    REQUIRE(count_expr != nullptr);
    REQUIRE(count_expr->val == "5");

    REQUIRE(dict_var.loc == Location{0, 0, 0});
}

TEST_CASE("Test parser with Config dictionary", "[parser]") {
    Parser parser(CaseLexemes::CONFIG_DICT);
    std::vector<ParsedVariable> parsed = parser.parse();

    REQUIRE(parsed.size() == 1);

    ParsedVariable& cfg_var = parsed.at(0);
    REQUIRE(cfg_var.identifier == "cfg");
    REQUIRE(cfg_var.category == VarCategory::CONFIG);
    DictionaryExpr* cfg_expr = dynamic_cast<DictionaryExpr*>(cfg_var.expr.get());
    REQUIRE(cfg_expr != nullptr);

    REQUIRE(cfg_expr->fields_map.size() == 4);
    REQUIRE(cfg_expr->fields_map.count("compiler") == 1);
    REQUIRE(cfg_expr->fields_map.count("compilation_flags") == 1);
    REQUIRE(cfg_expr->fields_map.count("link_flags") == 1);
    REQUIRE(cfg_expr->fields_map.count("default") == 1);

    StringExpr* compiler_expr =
        dynamic_cast<StringExpr*>(cfg_expr->fields_map.at("compiler").get());
    REQUIRE(compiler_expr != nullptr);
    REQUIRE(compiler_expr->val == "clang++");

    ListExpr* comp_flags_expr =
        dynamic_cast<ListExpr*>(cfg_expr->fields_map.at("compilation_flags").get());
    REQUIRE(comp_flags_expr != nullptr);
    REQUIRE(comp_flags_expr->elements.size() == 2);

    StringExpr* flag1 = dynamic_cast<StringExpr*>(comp_flags_expr->elements.at(0).get());
    REQUIRE(flag1 != nullptr);
    REQUIRE(flag1->val == "-g");

    StringExpr* flag2 = dynamic_cast<StringExpr*>(comp_flags_expr->elements.at(1).get());
    REQUIRE(flag2 != nullptr);
    REQUIRE(flag2->val == "-Wall");

    ListExpr* link_flags_expr =
        dynamic_cast<ListExpr*>(cfg_expr->fields_map.at("link_flags").get());
    REQUIRE(link_flags_expr != nullptr);
    REQUIRE(link_flags_expr->elements.size() == 0);

    StringExpr* default_expr = dynamic_cast<StringExpr*>(cfg_expr->fields_map.at("default").get());
    REQUIRE(default_expr != nullptr);
    REQUIRE(default_expr->val == "app");

    REQUIRE(cfg_var.loc == Location{0, 1, 0});
}

TEST_CASE("Test parser with Rule dictionary", "[parser]") {
    Parser parser(CaseLexemes::SINGLE_RULE_DICT);
    std::vector<ParsedVariable> parsed = parser.parse();

    REQUIRE(parsed.size() == 1);

    ParsedVariable& rule_var = parsed.at(0);
    REQUIRE(rule_var.identifier == "app");
    REQUIRE(rule_var.category == VarCategory::SINGLE_RULE);
    DictionaryExpr* rule_expr = dynamic_cast<DictionaryExpr*>(rule_var.expr.get());
    REQUIRE(rule_expr != nullptr);

    REQUIRE(rule_expr->fields_map.size() == 3);
    REQUIRE(rule_expr->fields_map.count("deps") == 1);
    REQUIRE(rule_expr->fields_map.count("output") == 1);
    REQUIRE(rule_expr->fields_map.count("step") == 1);

    ListExpr* deps_expr = dynamic_cast<ListExpr*>(rule_expr->fields_map.at("deps").get());
    REQUIRE(deps_expr != nullptr);
    REQUIRE(deps_expr->elements.size() == 2);

    StringExpr* dep1 = dynamic_cast<StringExpr*>(deps_expr->elements.at(0).get());
    REQUIRE(dep1 != nullptr);
    REQUIRE(dep1->val == "main.o");

    StringExpr* dep2 = dynamic_cast<StringExpr*>(deps_expr->elements.at(1).get());
    REQUIRE(dep2 != nullptr);
    REQUIRE(dep2->val == "utils.o");

    StringExpr* output_expr = dynamic_cast<StringExpr*>(rule_expr->fields_map.at("output").get());
    REQUIRE(output_expr != nullptr);
    REQUIRE(output_expr->val == "app");

    EnumExpr* step_expr = dynamic_cast<EnumExpr*>(rule_expr->fields_map.at("step").get());
    REQUIRE(step_expr != nullptr);
    REQUIRE(step_expr->scope == "Step");
    REQUIRE(step_expr->name == "LINK");

    REQUIRE(rule_var.loc == Location{0, 1, 0});
}

TEST_CASE("Test parser with MultiRule dictionary", "[parser]") {
    Parser parser(CaseLexemes::MULTI_RULE_DICT);
    std::vector<ParsedVariable> parsed = parser.parse();

    REQUIRE(parsed.size() == 1);

    ParsedVariable& rule_var = parsed.at(0);
    REQUIRE(rule_var.identifier == "compilation");
    REQUIRE(rule_var.category == VarCategory::MULTI_RULE);
    DictionaryExpr* rule_expr = dynamic_cast<DictionaryExpr*>(rule_var.expr.get());
    REQUIRE(rule_expr != nullptr);

    REQUIRE(rule_expr->fields_map.size() == 3);
    REQUIRE(rule_expr->fields_map.count("deps") == 1);
    REQUIRE(rule_expr->fields_map.count("output") == 1);
    REQUIRE(rule_expr->fields_map.count("step") == 1);

    VarRefExpr* deps_expr = dynamic_cast<VarRefExpr*>(rule_expr->fields_map.at("deps").get());
    REQUIRE(deps_expr != nullptr);
    REQUIRE(deps_expr->identifier == "cpp_files");

    VarRefExpr* output_expr = dynamic_cast<VarRefExpr*>(rule_expr->fields_map.at("output").get());
    REQUIRE(output_expr != nullptr);
    REQUIRE(output_expr->identifier == "cpp_names");

    EnumExpr* step_expr = dynamic_cast<EnumExpr*>(rule_expr->fields_map.at("step").get());
    REQUIRE(step_expr != nullptr);
    REQUIRE(step_expr->scope == "Step");
    REQUIRE(step_expr->name == "COMPILE");

    REQUIRE(rule_var.loc == Location{0, 1, 0});
}

TEST_CASE("Test parser with Clean dictionary", "[parser]") {
    Parser parser(CaseLexemes::CLEAN_DICT);
    std::vector<ParsedVariable> parsed = parser.parse();

    REQUIRE(parsed.size() == 1);

    ParsedVariable& clean_var = parsed.at(0);
    REQUIRE(clean_var.identifier == "clean_rule");
    REQUIRE(clean_var.category == VarCategory::CLEAN);
    DictionaryExpr* clean_expr = dynamic_cast<DictionaryExpr*>(clean_var.expr.get());
    REQUIRE(clean_expr != nullptr);

    REQUIRE(clean_expr->fields_map.size() == 1);
    REQUIRE(clean_expr->fields_map.count("targets") == 1);

    ListExpr* targets_expr = dynamic_cast<ListExpr*>(clean_expr->fields_map.at("targets").get());
    REQUIRE(targets_expr != nullptr);
    REQUIRE(targets_expr->elements.size() == 2);

    StringExpr* target1 = dynamic_cast<StringExpr*>(targets_expr->elements.at(0).get());
    REQUIRE(target1 != nullptr);
    REQUIRE(target1->val == "build/");

    StringExpr* target2 = dynamic_cast<StringExpr*>(targets_expr->elements.at(1).get());
    REQUIRE(target2 != nullptr);
    REQUIRE(target2->val == "*.o");

    REQUIRE(clean_var.loc == Location{0, 1, 0});
}