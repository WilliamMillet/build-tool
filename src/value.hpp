#ifndef VALUE_H
#define VALUE_H

#include <memory>
#include <string>
#include <variant>
#include <vector>

class Value;

using ValueList = std::vector<std::unique_ptr<Value>>;

struct ScopedEnumValue {
    std::string scope;
    std::string name;
};

enum class ValueType { INT, STRING, LIST, ENUM };

class Value {
   public:
    Value(int x);
    Value(std::string x);
    Value(ValueList x);
    Value(ScopedEnumValue x);

    template <typename T>
    const T& get() const {
        return std::get<T>(raw_val);
    }

    ValueType get_type() const;

   private:
    std::variant<int, std::string, ValueList, ScopedEnumValue> raw_val;
    ValueType type;
};

#endif