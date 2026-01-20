#include "src/lexer.hpp"

namespace CaseLexemes {
using Case = std::vector<Lexeme>;
// There are no original files, so we set the location to an index in the vector for easier debug

const Case EMPTY_FILE = {{LexemeType::END_OF_FILE, "", Location{0, 1, 0}}};

// word = "Apple"
// enum = Step::LINK
// my_list = ["Dog", "Cat"]
// EOF
const Case SIMPLE_VARS = {{LexemeType::IDENTIFIER, "word", Location{0, 0, 0}},
                          {LexemeType::EQUALS, "=", Location{0, 1, 0}},
                          {LexemeType::STRING, "Apple", Location{0, 2, 0}},
                          {LexemeType::NEWLINE, "\n", Location{0, 3, 0}},

                          {LexemeType::IDENTIFIER, "my_enum", Location{0, 4, 0}},
                          {LexemeType::EQUALS, "=", Location{0, 5, 0}},
                          {LexemeType::IDENTIFIER, "Step", Location{0, 6, 0}},
                          {LexemeType::SCOPE_RESOLVER, "::", Location{0, 7, 0}},
                          {LexemeType::IDENTIFIER, "LINK", Location{0, 8, 0}},
                          {LexemeType::NEWLINE, "\n", Location{0, 9, 0}},

                          {LexemeType::IDENTIFIER, "my_list", Location{0, 10, 0}},
                          {LexemeType::EQUALS, "=", Location{0, 11, 0}},
                          {LexemeType::LIST_START, "[", Location{0, 12, 0}},
                          {LexemeType::STRING, "Dog", Location{0, 13, 0}},
                          {LexemeType::DELIMETER, ",", Location{0, 14, 0}},
                          {LexemeType::STRING, "Cat", Location{0, 15, 0}},
                          {LexemeType::LIST_END, "]", Location{0, 16, 0}},
                          {LexemeType::NEWLINE, "\n", Location{0, 17, 0}},

                          {LexemeType::END_OF_FILE, "", Location{0, 18, 0}}};

// my_dict = {
//  name = "value"
//  count = "5"
// }
const Case REGULAR_DICT = {{LexemeType::IDENTIFIER, "my_dict", Location{0, 0, 0}},
                           {LexemeType::EQUALS, "=", Location{0, 1, 0}},
                           {LexemeType::BLOCK_START, "{", Location{0, 2, 0}},
                           {LexemeType::NEWLINE, "\n", Location{0, 3, 0}},
                           {LexemeType::IDENTIFIER, "name", Location{0, 4, 0}},
                           {LexemeType::EQUALS, "=", Location{0, 5, 0}},
                           {LexemeType::STRING, "value", Location{0, 6, 0}},
                           {LexemeType::NEWLINE, "\n", Location{0, 7, 0}},
                           {LexemeType::IDENTIFIER, "count", Location{0, 8, 0}},
                           {LexemeType::EQUALS, "=", Location{0, 9, 0}},
                           {LexemeType::STRING, "5", Location{0, 10, 0}},
                           {LexemeType::NEWLINE, "\n", Location{0, 11, 0}},
                           {LexemeType::BLOCK_END, "}", Location{0, 12, 0}},
                           {LexemeType::NEWLINE, "\n", Location{0, 13, 0}},
                           {LexemeType::END_OF_FILE, "", Location{0, 14, 0}}};

// <Config> cfg {
//     compiler = "clang++"
//     compilation_flags = ["-g", "-Wall"]
//     link_flags = []
//     default = "app"
// }
const Case CONFIG_DICT = {{LexemeType::DICT_QUALIFIER, "Config", Location{0, 0, 0}},
                          {LexemeType::IDENTIFIER, "cfg", Location{0, 1, 0}},
                          {LexemeType::BLOCK_START, "{", Location{0, 2, 0}},
                          {LexemeType::NEWLINE, "\n", Location{0, 3, 0}},

                          {LexemeType::IDENTIFIER, "compiler", Location{0, 4, 0}},
                          {LexemeType::EQUALS, "=", Location{0, 7, 0}},
                          {LexemeType::STRING, "clang++", Location{0, 8, 0}},
                          {LexemeType::NEWLINE, "\n", Location{0, 9, 0}},

                          {LexemeType::IDENTIFIER, "compilation_flags", Location{0, 10, 0}},
                          {LexemeType::EQUALS, "=", Location{0, 11, 0}},
                          {LexemeType::LIST_START, "[", Location{0, 12, 0}},
                          {LexemeType::STRING, "-g", Location{0, 13, 0}},
                          {LexemeType::DELIMETER, ",", Location{0, 14, 0}},
                          {LexemeType::STRING, "-Wall", Location{0, 15, 0}},
                          {LexemeType::LIST_END, "]", Location{0, 16, 0}},
                          {LexemeType::NEWLINE, "\n", Location{0, 17, 0}},

                          {LexemeType::IDENTIFIER, "link_flags", Location{0, 18, 0}},
                          {LexemeType::EQUALS, "=", Location{0, 19, 0}},
                          {LexemeType::LIST_START, "[", Location{0, 20, 0}},
                          {LexemeType::LIST_END, "]", Location{0, 21, 0}},
                          {LexemeType::NEWLINE, "\n", Location{0, 22, 0}},

                          {LexemeType::IDENTIFIER, "default", Location{0, 23, 0}},
                          {LexemeType::EQUALS, "=", Location{0, 24, 0}},
                          {LexemeType::STRING, "app", Location{0, 25, 0}},
                          {LexemeType::NEWLINE, "\n", Location{0, 26, 0}},

                          {LexemeType::BLOCK_END, "}", Location{0, 27, 0}},
                          {LexemeType::NEWLINE, "\n", Location{0, 28, 0}},
                          {LexemeType::END_OF_FILE, "", Location{0, 29, 0}}};

// <Rule> app {
//     deps = ["main.o", "utils.o"]
//     output = "app"
//     step = Step::LINK
// }
const Case SINGLE_RULE_DICT = {{LexemeType::DICT_QUALIFIER, "Rule", Location{0, 0, 0}},
                               {LexemeType::IDENTIFIER, "app", Location{0, 1, 0}},
                               {LexemeType::BLOCK_START, "{", Location{0, 2, 0}},
                               {LexemeType::NEWLINE, "\n", Location{0, 3, 0}},

                               {LexemeType::IDENTIFIER, "deps", Location{0, 4, 0}},
                               {LexemeType::EQUALS, "=", Location{0, 7, 0}},
                               {LexemeType::LIST_START, "[", Location{0, 8, 0}},
                               {LexemeType::STRING, "main.o", Location{0, 9, 0}},
                               {LexemeType::DELIMETER, ",", Location{0, 10, 0}},
                               {LexemeType::STRING, "utils.o", Location{0, 11, 0}},
                               {LexemeType::LIST_END, "]", Location{0, 12, 0}},
                               {LexemeType::NEWLINE, "\n", Location{0, 13, 0}},

                               {LexemeType::IDENTIFIER, "output", Location{0, 14, 0}},
                               {LexemeType::EQUALS, "=", Location{0, 15, 0}},
                               {LexemeType::STRING, "app", Location{0, 16, 0}},
                               {LexemeType::NEWLINE, "\n", Location{0, 17, 0}},

                               {LexemeType::IDENTIFIER, "step", Location{0, 18, 0}},
                               {LexemeType::EQUALS, "=", Location{0, 19, 0}},
                               {LexemeType::IDENTIFIER, "Step", Location{0, 20, 0}},
                               {LexemeType::SCOPE_RESOLVER, "::", Location{0, 21, 0}},
                               {LexemeType::IDENTIFIER, "LINK", Location{0, 22, 0}},
                               {LexemeType::NEWLINE, "\n", Location{0, 23, 0}},

                               {LexemeType::BLOCK_END, "}", Location{0, 24, 0}},
                               {LexemeType::NEWLINE, "\n", Location{0, 25, 0}},
                               {LexemeType::END_OF_FILE, "", Location{0, 26, 0}}};

// <MultiRule> compilation {
//     deps = cpp_files
//     output = cpp_names
//     step = Step::COMPILE
// }
const Case MULTI_RULE_DICT = {{LexemeType::DICT_QUALIFIER, "MultiRule", Location{0, 0, 0}},
                              {LexemeType::IDENTIFIER, "compilation", Location{0, 1, 0}},
                              {LexemeType::BLOCK_START, "{", Location{0, 2, 0}},
                              {LexemeType::NEWLINE, "\n", Location{0, 3, 0}},

                              {LexemeType::IDENTIFIER, "deps", Location{0, 4, 0}},
                              {LexemeType::EQUALS, "=", Location{0, 5, 0}},
                              {LexemeType::IDENTIFIER, "cpp_files", Location{0, 6, 0}},
                              {LexemeType::NEWLINE, "\n", Location{0, 7, 0}},

                              {LexemeType::IDENTIFIER, "output", Location{0, 8, 0}},
                              {LexemeType::EQUALS, "=", Location{0, 9, 0}},
                              {LexemeType::IDENTIFIER, "cpp_names", Location{0, 10, 0}},
                              {LexemeType::NEWLINE, "\n", Location{0, 11, 0}},

                              {LexemeType::IDENTIFIER, "step", Location{0, 12, 0}},
                              {LexemeType::EQUALS, "=", Location{0, 13, 0}},
                              {LexemeType::IDENTIFIER, "Step", Location{0, 14, 0}},
                              {LexemeType::SCOPE_RESOLVER, "::", Location{0, 15, 0}},
                              {LexemeType::IDENTIFIER, "COMPILE", Location{0, 16, 0}},
                              {LexemeType::NEWLINE, "\n", Location{0, 17, 0}},

                              {LexemeType::BLOCK_END, "}", Location{0, 18, 0}},
                              {LexemeType::NEWLINE, "\n", Location{0, 19, 0}},
                              {LexemeType::END_OF_FILE, "", Location{0, 20, 0}}};

// <Clean> clean_rule {
//     targets = ["build/", "*.o"]
// }
const Case CLEAN_DICT = {{LexemeType::DICT_QUALIFIER, "Clean", Location{0, 0, 0}},
                         {LexemeType::IDENTIFIER, "clean_rule", Location{0, 1, 0}},
                         {LexemeType::BLOCK_START, "{", Location{0, 2, 0}},
                         {LexemeType::NEWLINE, "\n", Location{0, 3, 0}},

                         {LexemeType::IDENTIFIER, "targets", Location{0, 4, 0}},
                         {LexemeType::EQUALS, "=", Location{0, 5, 0}},
                         {LexemeType::LIST_START, "[", Location{0, 6, 0}},
                         {LexemeType::STRING, "build/", Location{0, 7, 0}},
                         {LexemeType::DELIMETER, ",", Location{0, 8, 0}},
                         {LexemeType::STRING, "*.o", Location{0, 9, 0}},
                         {LexemeType::LIST_END, "]", Location{0, 10, 0}},
                         {LexemeType::NEWLINE, "\n", Location{0, 11, 0}},

                         {LexemeType::BLOCK_END, "}", Location{0, 12, 0}},
                         {LexemeType::NEWLINE, "\n", Location{0, 13, 0}},
                         {LexemeType::END_OF_FILE, "", Location{0, 14, 0}}};

// obj_names = file_names(["main.cpp", "utils.cpp"])
const Case FUNCTION_CALL = {{LexemeType::IDENTIFIER, "obj_names", Location{0, 0, 0}},
                            {LexemeType::EQUALS, "=", Location{0, 1, 0}},
                            {LexemeType::IDENTIFIER, "file_names", Location{0, 2, 0}},
                            {LexemeType::FN_START, "(", Location{0, 3, 0}},
                            {LexemeType::LIST_START, "[", Location{0, 4, 0}},
                            {LexemeType::STRING, "main.cpp", Location{0, 5, 0}},
                            {LexemeType::DELIMETER, ",", Location{0, 6, 0}},
                            {LexemeType::STRING, "utils.cpp", Location{0, 7, 0}},
                            {LexemeType::LIST_END, "]", Location{0, 8, 0}},
                            {LexemeType::FN_END, ")", Location{0, 9, 0}},
                            {LexemeType::NEWLINE, "\n", Location{0, 10, 0}},
                            {LexemeType::END_OF_FILE, "", Location{0, 11, 0}}};
}  // namespace CaseLexemes