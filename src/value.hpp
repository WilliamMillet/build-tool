#ifndef VALUE_H
#define VALUE_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class Value;

enum class ValueType { INT, STRING, LIST, ENUM, CFG_OBJ, NONE };

using ValTypePair = std::vector<std::pair<std::reference_wrapper<const Value>, ValueType>>;

class ValueList {
   public:
    std::vector<std::unique_ptr<Value>> elements;

    ValueList() = default;

    ValueList(const ValueList& other);

    ValueList& operator=(const ValueList& other);
};

struct ScopedEnumValue {
    std::string scope;
    std::string name;
};

class Dictionary {
   public:
    // No template method exists for get in this class as it would require Dictionary to be defined
    // after Value which is not possible due to the std::variant used

    /** Get a value from the dictionary */
    Value& get(const std::string& key);

    /** Return true if the dictionary contains a type */
    bool contains(const std::string& key) const;

    /** Add a key-value pair to the dictionary. Returns val */
    Value& insert(const std::string key, Value val);

    /**
     * Assert that the dictionary contains the a set of properties with defined types
     * @param shape A vector of (FieldName, ExpectedType) pairs
     * @note This method may be preferred over get + assert_type as it includes the field name in
     * exceptions
     */
    void assert_contains(const std::vector<std::pair<std::string, ValueType>> shape);

   private:
    std::unordered_map<std::string, Value> fields;
};

class Value {
   public:
    Value();
    Value(int x);
    Value(std::string&& x);
    Value(ValueList&& x);
    Value(ScopedEnumValue&& x);
    Value(Dictionary&& x);

    template <typename T>
    const T& get() const {
        return std::get<T>(raw_val);
    }

    ValueType get_type() const;

    /** Throw an exception if this value is not of an expected type */
    void assert_type(ValueType exp) const;

    Value& operator+=(const Value& other);

    /**
     * Throw an exception if a type does not match it's expected type
     * @param exp A vector of pairs from values to the expected types
     */
    static void assert_types(const ValTypePair exp);

   private:
    std::variant<int, std::string, ValueList, ScopedEnumValue, Dictionary> raw_val;
    ValueType type;

    inline static const std::unordered_map<ValueType, std::string> type_string_map = {
        {ValueType::INT, "Integer"},
        {ValueType::STRING, "String"},
        {ValueType::LIST, "List"},
        {ValueType::ENUM, "Enum"},
        {ValueType::NONE, "None"}};
};

#endif