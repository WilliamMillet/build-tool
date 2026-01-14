#ifndef NAMESPACE_BUILDER_H
#define NAMESPACE_BUILDER_H

#include <string>
#include <vector>

class NamespaceBuilder {
   public:
    NamespaceBuilder(char delimeter = DEFAULT_DELIMITER) : part_delimeter(delimeter) {};

    std::string curr_namespace() const;

    void append_identifier(std::string&& id);

    void pop_identifier();

    bool empty();

   private:
    constexpr static char DEFAULT_DELIMITER = '.';
    char part_delimeter;
    std::string namespace_str;
    std::vector<int> namespace_part_start_idxs;
};

#endif