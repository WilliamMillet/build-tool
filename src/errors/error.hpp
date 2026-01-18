#ifndef CUSTOM_ERR_H
#define CUSTOM_ERR_H

#include <exception>
#include <limits>
#include <optional>
#include <string>
#include <vector>

struct Location {
    constexpr static size_t END_OF_FILE = std::numeric_limits<size_t>::max();
    size_t line_no;
    size_t col_no;
    size_t file_idx;

    size_t line_start() const;

    bool is_eof() const;

    static Location eof_loc();
};

class Error : public std::exception {
   public:
    Error(std::string _msg, Location _loc);

    Error(std::string _msg = "");

    void set_loc(Location _loc);

    /** Add additional context as to the events occurring when the error was thrown */
    void add_ctx(std::string ctx);

    /** Returns true if and only if the location field has been set */
    bool has_loc() const;

    /**
     * Return a formatted error including the error type, an excerpt of the file where it occurred
     * and a trace of the events that occurred when thrown.
     * @param src_file The file the error occurred in (used to generate excerpt)
     */
    std::string format(const std::string& src_file) const;

    /**
     * If e is an Error, context will be added and the location will be written if not set, then
     * it will be rethrown
     * If e is not an Error, it will be converted to one with ctx and loc
     */
    [[noreturn]] static void update_and_throw(std::exception& excep, std::string ctx, Location loc);

   private:
    // A stack of events that were occurring when the error was thrown with increased
    // specificity (e.g. {"Parsing", "Parsing identifier 'name'"})
    std::vector<std::string> ctx_stack;
    std::optional<Location> loc;
    std::string msg;

    /** Get a formatted an excerpt from the codebase where the error occurred */
    std::string format_excerpt(const std::string& src_file) const;

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

#endif