#include "error.hpp"

#include <cstring>

#include "../file_utils.hpp"

size_t Location::line_start() const { return file_idx - col_no; }

bool Location::is_eof() const {
    return (line_no == END_OF_FILE) && (col_no == END_OF_FILE) && (file_idx == END_OF_FILE);
};

Location Location::eof_loc() { return Location{END_OF_FILE, END_OF_FILE, END_OF_FILE}; }

Error::Error(std::string _msg, Location _loc) : loc(_loc), msg(_msg) {};

Error::Error(std::string _msg) : msg(_msg) {};

void Error::set_loc(Location _loc) { loc = _loc; };

void Error::add_ctx(std::string ctx) { ctx_stack.push_back(ctx); };

bool Error::has_loc() const { return loc.has_value(); };

std::string Error::format(const std::string& src_file) const {
    std::string err = "Exception thrown: " + err_name();
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
    std::vector<std::string> chunk;
    try {
        chunk = FileUtils::read_chunk(src_file, loc->line_start());
    } catch (const std::exception& e) {
        std::string err_msg = "Failed to read code excerpt";
        err_msg += e.what();
        return err_msg;
    }

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

void Error::update_and_throw(std::exception& excep, std::string ctx, Location loc) {
    if (Error* err = dynamic_cast<Error*>(&excep)) {
        err->add_ctx(ctx);
        if (!err->has_loc()) {
            err->loc = loc;
        }
        throw;
    }

    throw UnknownError(excep, ctx, loc);
}

void Error::update_and_throw(std::exception& excep, std::string ctx) {
    if (Error* err = dynamic_cast<Error*>(&excep)) {
        err->add_ctx(ctx);
        throw;
    }

    throw UnknownError(excep, ctx);
}

UnknownError::UnknownError(std::string _msg, Location _loc) : Error(_msg, _loc) {}
UnknownError::UnknownError(std::string _msg) : Error(_msg) {}
std::string UnknownError::err_name() const { return "UnknownError"; }

UnknownError::UnknownError(const std::exception& excep, std::string ctx, Location _loc)
    : Error(excep.what(), _loc) {
    add_ctx(ctx);
};

UnknownError::UnknownError(const std::exception& excep, std::string ctx) : Error(excep.what()) {
    add_ctx(ctx);
};

// TODO - Consider deleting this IOError im not using it anywhere. Either that or user it
IOError::IOError(std::string _msg, Location _loc) : Error(_msg, _loc) {}
IOError::IOError(std::string _msg) : Error(_msg) {}
std::string IOError::err_name() const { return "IOError"; }

SyntaxError::SyntaxError(std::string _msg, Location _loc) : Error(_msg, _loc) {}
SyntaxError::SyntaxError(std::string _msg) : Error(_msg) {}
std::string SyntaxError::err_name() const { return "SyntaxError"; }

TypeError::TypeError(std::string _msg, Location _loc) : Error(_msg, _loc) {}
TypeError::TypeError(std::string _msg) : Error(_msg) {}
std::string TypeError::err_name() const { return "TypeError"; }

ValueError::ValueError(std::string _msg, Location _loc) : Error(_msg, _loc) {}
ValueError::ValueError(std::string _msg) : Error(_msg) {}
std::string ValueError::err_name() const { return "ValueError"; }

LogicError::LogicError(std::string _msg, Location _loc) : Error(_msg, _loc) {}
LogicError::LogicError(std::string _msg) : Error(_msg) {}
std::string LogicError::err_name() const { return "LogicError"; }

SystemError::SystemError(std::string _msg, Location _loc)
    : Error(std::string(std::strerror(errno)) + "." + _msg, _loc) {}
SystemError::SystemError(std::string _msg)
    : Error(std::string(std::strerror(errno)) + "." + _msg) {}
std::string SystemError::err_name() const { return "LogicError"; }
