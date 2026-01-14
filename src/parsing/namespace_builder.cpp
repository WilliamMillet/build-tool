#include "namespace_builder.hpp"

std::string NamespaceBuilder::curr_namespace() const { return namespace_str; }

void NamespaceBuilder::append_identifier(std::string&& id) {
    namespace_part_start_idxs.push_back(namespace_str.size());

    if (!namespace_str.empty()) {
        namespace_str += part_delimeter;
    }
    namespace_str += std::to_string(namespace_id++);
    namespace_str += len_postfix;

    namespace_str += id;
};

void NamespaceBuilder::pop_identifier() {
    namespace_str.erase(namespace_part_start_idxs.back());
    namespace_part_start_idxs.pop_back();
}

bool NamespaceBuilder::empty() { return namespace_str.empty(); }