#include <exception>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

struct Location {
    size_t line_no;
    size_t col_no;
    size_t file_idx;

    size_t line_start() const { return file_idx - col_no; }
};

enum class ErrorType {
    UNKNOWN_ERROR,
    IO_ERROR,
    // The syntax is malformed and cannot be parsed
    SYNTAX_ERROR,
    // The wrong type is given (e.g. List is used over Map)
    TYPE_ERROR,
    // The correct type is given, but the value itself is wrong
    VALUE_ERROR,
};

class Error : public std::exception {
   public:
    Error(ErrorType _type, std::string _msg, Location _loc);

    Error(ErrorType _type, std::string _msg = "");

    void add_ctx(std::string ctx);

    void set_loc(Location _loc);

    std::string format(const std::string& src_file) const;

    /** Get a formatted an excerpt from the codebase where the error occurred */
    std::string format_excerpt(const std::string& src_file) const;

   private:
    std::string msg;
    ErrorType type;
    // A stack of processes that were occurring when the error was thrown with increased
    // specificity (e.g. {"Parsing", "Parsing identifier 'name'"})
    std::vector<std::string> ctx_stack;

    std::optional<Location> loc;

    constexpr static std::string err_type_str(ErrorType err_type) {
        switch (err_type) {
            case ErrorType::IO_ERROR:
                return "IoError";
            case ErrorType::SYNTAX_ERROR:
                return "SyntaxError";
            case ErrorType::TYPE_ERROR:
                return "TypeError";
            case ErrorType::VALUE_ERROR:
                return "ValueError";
            case ErrorType::UNKNOWN_ERROR:
                return "UnknownError";
            default:
                throw std::invalid_argument("Unknown error type");
        }
    }
};