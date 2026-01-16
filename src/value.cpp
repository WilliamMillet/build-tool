#include "value.hpp"

#include <stdexcept>

ValueList::ValueList(const ValueList& other) {
    elements.reserve(other.elements.size());
    for (const std::unique_ptr<Value>& v : other.elements) {
        elements.push_back(std::make_unique<Value>(*v));
    }
};

ValueList& ValueList::operator=(const ValueList& other) {
    if (this == &other) return *this;
    ValueList tmp(other);
    elements.swap(tmp.elements);
    return *this;
}

Value& Dictionary::get(const std::string& key) { return fields.at(key); }

bool Dictionary::contains(const std::string& key) const { return fields.contains(key); }

Value& Dictionary::insert(const std::string key, Value val) { return fields[key] = val; }

void Dictionary::assert_contains(const std::vector<std::pair<std::string, ValueType>> shape) {
    for (const auto& [field, field_type] : shape) {
        auto itm = fields.find(field);
        if (itm == fields.end()) {
            throw std::invalid_argument("Dictionary missing expected field '" + field + "'");
        }
        try {
            itm->second.assert_type(field_type);
        } catch (std::invalid_argument err) {
            throw std::invalid_argument("Failed to parse dictionary field '" + field +
                                        "': " + err.what());
        }
    }
}

Value::Value() { type = ValueType::NONE; }

Value::Value(int x) : raw_val(x) { type = ValueType::INT; }

Value::Value(std::string&& x) : raw_val(std::move(x)) { type = ValueType::STRING; }

Value::Value(ValueList&& x) : raw_val(std::move(x)) { type = ValueType::LIST; }

Value::Value(ScopedEnumValue&& x) : raw_val(std::move(x)) { type = ValueType::ENUM; }

Value::Value(Dictionary&& x) : raw_val(std::move(x)) { type = ValueType::CFG_OBJ; }

ValueType Value::get_type() const { return type; }

Value& Value::operator+=(const Value& other) {
    if (other.type != type) {
        const std::string type_a = type_string_map.at(type);
        const std::string type_b = type_string_map.at(type);
        throw std::invalid_argument("Cannot add two values of distinct types ('" + type_a +
                                    "' + '" + type_b + "')");
    }

    switch (type) {
        case ValueType::INT: {
            raw_val = std::get<int>(raw_val) + std::get<int>(other.raw_val);
            break;
        }
        case ValueType::STRING: {
            raw_val = std::get<std::string>(raw_val) + std::get<std::string>(other.raw_val);
            break;
        }
        case ValueType::LIST: {
            auto& base_list = std::get<ValueList>(raw_val).elements;
            for (const auto& v : std::get<ValueList>(other.raw_val).elements) {
                base_list.push_back(std::make_unique<Value>(*v));
            }
            break;
        }
        default: {
            std::string type_name = type_string_map.at(type);
            throw std::invalid_argument("Type '" + type_name + "' does not support addition");
        }
    }

    return *this;
}

void Value::assert_type(ValueType exp) const {
    if (type != exp) {
        throw std::invalid_argument("Expected type '" + type_string_map.at(exp) +
                                    "' but got type '" + type_string_map.at(exp) + "'");
    }
}

void Value::assert_types(const ValTypePair exp) {
    for (const auto& [got, exp_type] : exp) {
        got.get().assert_type(exp_type);
    }
}