#include "value.hpp"

#include <stdexcept>

#include "errors/error.hpp"

ValueList::ValueList(std::vector<std::unique_ptr<Value>> elems) : elements(std::move(elems)) {};

size_t ValueList::size() const { return elements.size(); }

ValueList::ValueList(const ValueList& other) {
    elements.reserve(other.elements.size());
    for (const Value& v : other) {
        elements.push_back(std::make_unique<Value>(v));
    }
};

ValueList& ValueList::operator=(const ValueList& other) {
    if (this == &other) return *this;
    ValueList tmp(other);
    elements.swap(tmp.elements);
    return *this;
}

ValueList& ValueList::operator+=(const ValueList& other) {
    for (const Value& v : other) {
        elements.push_back(std::make_unique<Value>(v));
    }
    return *this;
}

ValueList::iterator ValueList::begin() { return ValueList::iterator(elements.begin()); };
ValueList::iterator ValueList::end() { return ValueList::iterator(elements.end()); };

ValueList::const_iterator ValueList::begin() const {
    return ValueList::const_iterator(elements.cbegin());
};
ValueList::const_iterator ValueList::end() const {
    return ValueList::const_iterator(elements.cend());
};

Value& Dictionary::get(const std::string& key) { return fields.at(key); }

bool Dictionary::contains(const std::string& key) const { return fields.contains(key); }

Value& Dictionary::insert(const std::string key, Value val) { return fields[key] = val; }

void Dictionary::assert_contains(const std::vector<std::pair<std::string, ValueType>> shape) {
    for (const auto& [field, field_type] : shape) {
        auto itm = fields.find(field);
        if (itm == fields.end()) {
            throw ValueError("Dictionary missing expected field '" + field + "'");
        }

        try {
            itm->second.assert_type(field_type);
        } catch (const std::invalid_argument& err) {
            throw ValueError("Failed to parse dictionary field '" + field + "': " + err.what());
        }
    }
}

Value::Value() { type = ValueType::NONE; }

Value::Value(int x) : raw_val(x) { type = ValueType::INT; }

Value::Value(std::string x) : raw_val(std::move(x)) { type = ValueType::STRING; }

Value::Value(ValueList x) : raw_val(std::move(x)) { type = ValueType::LIST; }

Value::Value(ScopedEnumValue x) : raw_val(std::move(x)) { type = ValueType::ENUM; }

Value::Value(Dictionary x) : raw_val(std::move(x)) { type = ValueType::Dictionary; }

ValueType Value::get_type() const { return type; }

Value& Value::operator+=(const Value& other) {
    if (other.type != type) {
        const std::string type_a = type_string_map.at(type);
        const std::string type_b = type_string_map.at(type);
        throw TypeError("Cannot add two values of distinct types ('" + type_a + "' + '" + type_b +
                        "')");
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
            std::get<ValueList>(raw_val) += std::get<ValueList>(other.raw_val);
            break;
        }
        default: {
            std::string type_name = type_string_map.at(type);
            throw TypeError("Type '" + type_name + "' does not support addition");
        }
    }

    return *this;
}

void Value::assert_type(ValueType exp) const {
    if (type != exp) {
        throw TypeError("Expected type '" + type_string_map.at(exp) + "' but got type '" +
                        type_string_map.at(type) + "'");
    }
}

void Value::assert_types(const ValTypePair exp) {
    for (const auto& [got, exp_type] : exp) {
        got.get().assert_type(exp_type);
    }
}