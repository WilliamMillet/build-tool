#ifndef VALUE_H
#define VALUE_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class Value;

struct ValueList {
    std::vector<std::unique_ptr<Value>> elements;

    ValueList() = default;

    ValueList(const ValueList& other);

    ValueList& operator=(const ValueList& other);
};

struct ScopedEnumValue {
    std::string scope;
    std::string name;
};

struct ConfigObj {
    std::unordered_map<std::string, Value> fields;
};

enum class ValueType { INT, STRING, LIST, ENUM, CFG_OBJ, NONE };

using AssertionPair = std::vector<std::pair<std::reference_wrapper<const Value>, ValueType>>;

class Value {
   public:
    Value();
    Value(int x);
    Value(std::string&& x);
    Value(ValueList&& x);
    Value(ScopedEnumValue&& x);
    Value(ConfigObj&& x);

    template <typename T>
    const T& get() const {
        return std::get<T>(raw_val);
    }

    ValueType get_type() const;

    Value& operator+=(const Value& other);

    /**
     * Throw an exception if a type does not match it's expected type
     * @param exp A vector of pairs from values to the expected types
     */
    static void assert_types(const AssertionPair exp);

   private:
    std::variant<int, std::string, ValueList, ScopedEnumValue, ConfigObj> raw_val;
    ValueType type;

    inline static const std::unordered_map<ValueType, std::string> type_string_map = {
        {ValueType::INT, "Integer"},
        {ValueType::STRING, "String"},
        {ValueType::LIST, "List"},
        {ValueType::ENUM, "Enum"},
        {ValueType::NONE, "None"}};
};

#endif