#include "value.hpp"

Value::Value(int x) : raw_val(x) { this->type = ValueType::INT; };

Value::Value(std::string x) : raw_val(std::move(x)) { this->type = ValueType::STRING; };

Value::Value(ValueList x) : raw_val(std::move(x)) { this->type = ValueType::LIST; };

Value::Value(ScopedEnumValue x) : raw_val(std::move(x)) { this->type = ValueType::ENUM; };

ValueType Value::get_type() const { return this->type; };