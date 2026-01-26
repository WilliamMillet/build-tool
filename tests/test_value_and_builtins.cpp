/** Tests for the Value class and built-in functions */
#include <filesystem>
#include <memory>
#include <unordered_set>
#include <vector>

#include "../catch.hpp"
#include "src/built_in/func_registry.hpp"
#include "src/built_in/funcs.hpp"
#include "src/value.hpp"
#include "utils.hpp"

// Tests for value type

TEST_CASE("String Value stores and retrieves correctly", "[value]") {
    Value str_val = std::string("Hello World");

    REQUIRE(str_val.get_type() == ValueType::STRING);
    REQUIRE(str_val.get<std::string>() == "Hello World");
}

TEST_CASE("Integer Value stores and retrieves correctly", "[value]") {
    Value int_val = 42;

    REQUIRE(int_val.get_type() == ValueType::INT);
    REQUIRE(int_val.get<int>() == 42);
}

TEST_CASE("Enum Value stores and retrieves correctly", "[value]") {
    Value enum_val = ScopedEnumValue{"Step", "COMPILE"};

    REQUIRE(enum_val.get_type() == ValueType::ENUM);
    const auto& scoped = enum_val.get<ScopedEnumValue>();
    REQUIRE(scoped.scope == "Step");
    REQUIRE(scoped.name == "COMPILE");
}

TEST_CASE("ValueList stores and retrieves correctly", "[value]") {
    std::vector<std::unique_ptr<Value>> items;
    items.push_back(std::make_unique<Value>(std::string("item1")));
    items.push_back(std::make_unique<Value>(std::string("item2")));

    ValueList list(std::move(items));
    Value list_val = list;

    REQUIRE(list_val.get_type() == ValueType::LIST);
}

TEST_CASE("Empty ValueList is valid", "[value]") {
    std::vector<std::unique_ptr<Value>> empty;
    ValueList list(std::move(empty));
    Value list_val = list;

    REQUIRE(list_val.get_type() == ValueType::LIST);
}

// Tests for value type assertions

TEST_CASE("assert_type passes for correct type", "[value][assertions]") {
    Value str_val = std::string("test");

    REQUIRE_NOTHROW(str_val.assert_type(ValueType::STRING));
}

TEST_CASE("assert_type throws for incorrect type", "[value][assertions]") {
    Value str_val = std::string("test");

    REQUIRE_THROWS(str_val.assert_type(ValueType::INT));
    REQUIRE_THROWS(str_val.assert_type(ValueType::LIST));
    REQUIRE_THROWS(str_val.assert_type(ValueType::ENUM));
}

TEST_CASE("assert_types passes for multiple correct types", "[value][assertions]") {
    Value str_val = std::string("test");
    Value int_val = 42;

    ValTypePair pairs = {{str_val, ValueType::STRING}, {int_val, ValueType::INT}};

    REQUIRE_NOTHROW(Value::assert_types(pairs));
}

TEST_CASE("assert_types throws when any type is incorrect", "[value][assertions]") {
    Value str_val = std::string("test");
    Value int_val = 42;

    ValTypePair pairs = {
        {str_val, ValueType::STRING}, {int_val, ValueType::STRING}  // Wrong type
    };

    REQUIRE_THROWS(Value::assert_types(pairs));
}

// Tests for value operations

TEST_CASE("String concatenation with operator+=", "[value][operations]") {
    Value a = std::string("Hello ");
    Value b = std::string("World");

    a += b;

    REQUIRE(a.get<std::string>() == "Hello World");
}

TEST_CASE("ValueList concatenation with operator+=", "[value][operations]") {
    std::vector<std::unique_ptr<Value>> items1;
    items1.push_back(std::make_unique<Value>(std::string("a")));
    items1.push_back(std::make_unique<Value>(std::string("b")));

    std::vector<std::unique_ptr<Value>> items2;
    items2.push_back(std::make_unique<Value>(std::string("c")));
    items2.push_back(std::make_unique<Value>(std::string("d")));

    ValueList list1(std::move(items1));
    ValueList list2(std::move(items2));

    list1 += list2;

    // Verify by iterating
    int count = 0;
    for (const auto& val : list1) {
        count++;
        REQUIRE(val.get_type() == ValueType::STRING);
    }
    REQUIRE(count == 4);
}

// Tests for dictionaries

TEST_CASE("Dictionary insert and get", "[value][dictionary]") {
    Dictionary dict;

    dict.insert("name", Value(std::string("test")));
    dict.insert("count", Value(42));

    REQUIRE(dict.contains("name"));
    REQUIRE(dict.contains("count"));
    REQUIRE_FALSE(dict.contains("unknown"));

    REQUIRE(dict.get("name").get<std::string>() == "test");
    REQUIRE(dict.get("count").get<int>() == 42);
}

TEST_CASE("Dictionary assert_contains passes for valid shape", "[value][dictionary]") {
    Dictionary dict;
    dict.insert("name", Value(std::string("test")));
    dict.insert("value", Value(42));

    std::vector<std::pair<std::string, ValueType>> shape = {{"name", ValueType::STRING},
                                                            {"value", ValueType::INT}};

    REQUIRE_NOTHROW(dict.assert_contains(shape));
}

TEST_CASE("Dictionary assert_contains throws for missing field", "[value][dictionary]") {
    Dictionary dict;
    dict.insert("name", Value(std::string("test")));

    std::vector<std::pair<std::string, ValueType>> shape = {
        {"name", ValueType::STRING}, {"missing", ValueType::INT}  // Not in dict
    };

    REQUIRE_THROWS(dict.assert_contains(shape));
}

TEST_CASE("Dictionary assert_contains throws for wrong type", "[value][dictionary]") {
    Dictionary dict;
    dict.insert("name", Value(std::string("test")));

    std::vector<std::pair<std::string, ValueType>> shape = {
        {"name", ValueType::INT}  // Wrong type
    };

    REQUIRE_THROWS(dict.assert_contains(shape));
}

// Tests for value utils

TEST_CASE("vectorise converts ValueList to vector of strings", "[value][utils]") {
    std::vector<std::unique_ptr<Value>> items;
    items.push_back(std::make_unique<Value>(std::string("first")));
    items.push_back(std::make_unique<Value>(std::string("second")));
    items.push_back(std::make_unique<Value>(std::string("third")));

    ValueList list(std::move(items));

    std::vector<std::string> result = ValueUtils::vectorise<std::string>(list);

    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == "first");
    REQUIRE(result[1] == "second");
    REQUIRE(result[2] == "third");
}

// Tests for built in functions

TEST_CASE("file_names strips extensions correctly", "[builtins]") {
    std::vector<std::unique_ptr<Value>> files;
    files.push_back(std::make_unique<Value>(std::string("main.cpp")));
    files.push_back(std::make_unique<Value>(std::string("utils.cpp")));
    files.push_back(std::make_unique<Value>(std::string("parser.hpp")));

    ValueList file_list(std::move(files));
    std::vector<Value> args = {Value(file_list)};

    Value result = BuiltIn::file_names(args);

    REQUIRE(result.get_type() == ValueType::LIST);
}

// Tests for function registry

TEST_CASE("FuncRegistry can call file_names function", "[builtins][registry]") {
    FuncRegistry registry;

    std::vector<std::unique_ptr<Value>> files;
    files.push_back(std::make_unique<Value>(std::string("test.cpp")));
    ValueList file_list(std::move(files));

    std::vector<Value> args = {Value(file_list)};

    Value result = registry.call("file_names", args);

    REQUIRE(result.get_type() == ValueType::LIST);
}

TEST_CASE("FuncRegistry throws on unknown function", "[builtins][registry]") {
    FuncRegistry registry;
    std::vector<Value> args;

    REQUIRE_THROWS(registry.call("unknown_func", args));
}

// 'files' function tests

TEST_CASE("Files function works for flat directories", "[builtins][files]") {
    FuncRegistry registry;

    std::filesystem::path path_end = "code";
    path_end /= "flat";
    const std::filesystem::path path = IO::get_test_file_path(path_end);

    const std::vector<Value> args = {Value(path)};
    const Value result = registry.call("files", args);

    REQUIRE(result.get_type() == ValueType::LIST);
    const ValueList& vlist = result.get<ValueList>();

    std::unordered_set<std::string> files_found;

    for (const Value& v : vlist) {
        REQUIRE(v.get_type() == ValueType::STRING);
        files_found.insert(v.get<std::string>());
    }

    REQUIRE(files_found.size() == 2);
    REQUIRE(files_found.contains("a.cpp"));
    REQUIRE(files_found.contains("b.cpp"));
}

TEST_CASE("Files function ignores not .cpp files", "[builtins][files]") {
    FuncRegistry registry;

    std::filesystem::path path_end = "code";
    path_end /= "headers";
    const std::filesystem::path path = IO::get_test_file_path(path_end);

    const std::vector<Value> args = {Value(path)};
    const Value result = registry.call("files", args);

    REQUIRE(result.get_type() == ValueType::LIST);
    const ValueList& vlist = result.get<ValueList>();

    std::unordered_set<std::string> files_found;

    REQUIRE(vlist.size() == 0);
}

TEST_CASE("Files function works recursively", "[builtins][files]") {
    FuncRegistry registry;

    std::filesystem::path path_end = "code";
    path_end /= "nested";
    const std::filesystem::path path = IO::get_test_file_path(path_end);

    const std::vector<Value> args = {Value(path)};
    const Value result = registry.call("files", args);

    REQUIRE(result.get_type() == ValueType::LIST);
    const ValueList& vlist = result.get<ValueList>();

    std::unordered_set<std::string> files_found;

    for (const Value& v : vlist) {
        REQUIRE(v.get_type() == ValueType::STRING);
        files_found.insert(v.get<std::string>());
    }

    REQUIRE(files_found.size() == 3);

    REQUIRE(files_found.contains("a.cpp"));
    REQUIRE(files_found.contains("b.cpp"));
    REQUIRE(files_found.contains("c.cpp"));
}
