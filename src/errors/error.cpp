#include "error.hpp"

#include "../file_utils.hpp"

Error::Error(ErrorType _type, std::string _msg, Location _loc)
    : msg(_msg), type(_type), loc(_loc) {};

Error::Error(ErrorType _type, std::string _msg) : msg(_msg), type(_type) {};

void Error::add_ctx(std::string ctx) { ctx_stack.push_back(ctx); };

void Error::set_loc(Location _loc) { loc = _loc; };

std::string Error::format(const std::string& src_file) const {
    std::string err = "Exception thrown: " + err_type_str(type);
    err += "\nMessage: " + msg;

    if (loc.has_value()) {
        const std::string pos = std::to_string(loc->line_no) + ":" + std::to_string(loc->col_no);

        err += "\nLocation: " + src_file + ":" + pos + ":\n";
        err += format_excerpt(src_file);
    }

    if (!ctx_stack.empty()) {
        err += "\nContext: Error occurred during:";
    }
    for (const std::string& ctx : ctx_stack) {
        err += "\n- [" + ctx + "]";
    }

    return err;
}

std::string Error::format_excerpt(const std::string& src_file) const {
    const std::vector<std::string> chunk = FileUtils::read_chunk(src_file, loc->line_start());

    const std::string initial_lno = std::to_string(loc->line_no);
    const std::string whitespace_prefix = std::string(initial_lno.size(), ' ');

    std::string formatted;
    size_t relative_lno = 1;
    for (const std::string& line : chunk) {
        if (relative_lno == 1) {
            formatted += initial_lno;
        } else {
            formatted += whitespace_prefix;
        }
        formatted += " |";
        formatted += line;

        if (relative_lno == chunk.size()) {
            break;
        }

        formatted += '\n';

        formatted += whitespace_prefix;
        formatted += " |";
        if (relative_lno == 1) {
            formatted += std::string(loc->col_no, ' ');
            formatted += "^ error here";
        }

        formatted += "\n";
        relative_lno++;
    }

    return formatted;
}