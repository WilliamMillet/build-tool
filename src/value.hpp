#ifndef VALUE_H
#define VALUE_H

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

class Value;

enum class ValueType { INT, STRING, LIST, ENUM, Dictionary, NONE };

using ValTypePair = std::vector<std::pair<std::reference_wrapper<const Value>, ValueType>>;

class ValueList {
   public:
    ValueList() = default;

    ValueList(std::vector<std::unique_ptr<Value>> elems);

    ValueList(const ValueList& other);

    ValueList& operator=(const ValueList& other);

    ValueList& operator+=(const ValueList& other);

    template <typename T>
    class ValueIterator {
        using ConstIter = std::vector<std::unique_ptr<Value>>::const_iterator;
        using RegIter = std::vector<std::unique_ptr<Value>>::iterator;
        using BaseIter = std::conditional<std::is_const_v<T>, ConstIter, RegIter>::type;

       public:
        explicit ValueIterator(BaseIter iter) : curr(iter) {};

        ValueIterator& operator++() {
            curr++;
            return *this;
        }

        bool operator!=(const ValueIterator& other) const { return curr != other.curr; }

        T& operator*() const { return **curr; }

        T* operator->() const { return curr->get(); };

       private:
        BaseIter curr;
    };

    using iterator = ValueIterator<Value>;
    using const_iterator = ValueIterator<const Value>;

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

   private:
    std::vector<std::unique_ptr<Value>> elements;
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
    Value(std::string x);
    Value(ValueList x);
    Value(ScopedEnumValue x);
    Value(Dictionary x);

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

namespace ValueUtils {
/**
 * Get a vectorised list from a ValueList
 * @tparam A non-recursive Value underlying type (e.g. not a map or vector). Default is string as
 * that is by far the most common in this build system
 * @param match The type each element of the list must match
 * @note This cannot be a method of ValueList right now due to incomplete definition issues
 */
template <typename T>
std::vector<T> vectorise(ValueList vl, ValueType match = ValueType::STRING) {
    std::vector<T> vec;
    for (Value& v : vl) {
        v.assert_type(match);
        vec.push_back(std::move(v.get<T>()));
    }
    return vec;
}

}  // namespace ValueUtils

#endif