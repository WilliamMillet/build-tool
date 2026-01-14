#ifndef NAMESPACE_BUILDER_H
#define NAMESPACE_BUILDER_H

#include <string>
#include <vector>

class NamespaceBuilder {
   public:
    NamespaceBuilder(char _part_delimeter = DEFAULT_TOK_DELIMITER,
                     char _len_postfix = DEFAULT_LEN_POSTFIX)
        : part_delimeter(_part_delimeter), len_postfix(_len_postfix) {};

    std::string curr_namespace() const;

    void append_identifier(std::string&& id);

    void pop_identifier();

    bool empty();

   private:
    constexpr static char DEFAULT_TOK_DELIMITER = '.';
    constexpr static char DEFAULT_LEN_POSTFIX = '_';

    char part_delimeter;
    char len_postfix;
    size_t namespace_id = 0;
    std::string namespace_str;
    std::vector<int> namespace_part_start_idxs;
};

#endif