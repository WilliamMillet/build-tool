#ifndef CUSTOM_ERR_H
#define CUSTOM_ERR_H

#include <exception>
#include <limits>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

struct Location {
    size_t line_no;
    size_t col_no;
    size_t file_idx;

    constexpr static size_t END_OF_FILE = std::numeric_limits<size_t>::max();

    size_t line_start() const;

    bool is_eof() const;

    bool operator==(const Location&) const = default;

    friend std::ostream& operator<<(std::ostream& os, const Location& loc) {
        os << "Location{line_no = " << loc.line_no << ", col_no = " << loc.col_no
           << ", file_idx = " << loc.file_idx << "}";
        return os;
    }

    static Location eof_loc();
};

class Error : public std::exception {
   public:
    Error(std::string _msg, Location _loc);

    Error(std::string _msg = "");

    void set_loc(Location _loc);

    /** Set the string that is used when the what member is called*/
    void set_what_str(std::string msg);

    const char* what() const noexcept override;

    /** Add additional context as to the events occurring when the error was thrown */
    void add_ctx(std::string ctx);

    /** Returns true if and only if the location field has been set */
    bool has_loc() const;

    /**
     * Return a formatted error including the error type, an excerpt of the file where it occurred
     * and a trace of the events that occurred when thrown.
     * @param src_file The file the error occurred in (used to generate excerpt)
     * @returns The formatted except
     */
    std::string format(const std::string& src_file) const;

    /** Formats without file name and excerpt */
    std::string format() const;

    /**
     * Take either an Error or unknown exception, update it by adding a location if possible, then
     * propagate it
     * @param excep The original thrown exception. Either an Error, or unknown exception
     * @param ctx Some context to add to the error (e.g. it occurred during this parsing step)
     * @param loc Location data about the error for debugging
     * @throws a propagated Error
     */
    [[noreturn]] static void update_and_throw(std::exception& excep, std::string ctx, Location loc);

    [[noreturn]] static void update_and_throw(std::exception& excep, std::string ctx);

   private:
    // A stack of events that were occurring when the error was thrown with increased
    // specificity (e.g. {"Parsing", "Parsing identifier 'name'"})
    std::vector<std::string> ctx_stack;
    std::optional<Location> loc;
    std::string msg;

    std::string what_str;

    /** Get a formatted an excerpt from the codebase where the error occurred */
    std::string format_excerpt(const std::string& src_file) const;

    /** Get a formatted representation of the context */
    std::string format_ctx() const;

    /** Format the position in the file to "[line]:[col]" (no square brackets) */
    std::string format_file_pos() const;

    /** Get the name of the error type (e.g. "UnknownError", "IOError") */
    virtual std::string err_name() const;
};

/** It is not known why the error occurred */
class UnknownError : public Error {
   public:
    UnknownError(std::string _msg, Location _loc);
    UnknownError(std::string _msg = "");

    UnknownError(const std::exception& excep, std::string ctx, Location loc);
    UnknownError(const std::exception& excep, std::string ctx);

    std::string err_name() const override;
};

/** Failed interaction with files */
class IOError : public Error {
   public:
    IOError(std::string _msg, Location _loc);
    IOError(std::string _msg = "");

    std::string err_name() const override;
};

/** Parsing failed as the syntax is malformed */
class SyntaxError : public Error {
   public:
    SyntaxError(std::string _msg, Location _loc);
    SyntaxError(std::string _msg = "");

    std::string err_name() const override;
};

/** The wrong type is given (e.g. List is used over Map) */
class TypeError : public Error {
   public:
    TypeError(std::string _msg, Location _loc);
    TypeError(std::string _msg = "");

    std::string err_name() const override;
};

/** The correct type is given, but the value itself is wrong  */
class ValueError : public Error {
   public:
    ValueError(std::string _msg, Location _loc);
    ValueError(std::string _msg = "");

    std::string err_name() const override;
};

/** There is a logical issue with the program. E.g. cyclical dependencies  */
class LogicError : public Error {
   public:
    LogicError(std::string _msg, Location _loc);
    LogicError(std::string _msg = "");

    std::string err_name() const override;
};

/**
 * External error related to the system itself like a process failing to spawn. The message
 * associated with the errno value at the time of construction will be used in addition to any user
 * provided message.
 */
class SystemError : public Error {
   public:
    SystemError(std::string _msg, Location _loc);
    SystemError(std::string _msg = "");

    std::string err_name() const override;
};

#endif