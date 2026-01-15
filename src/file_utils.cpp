#include "file_utils.hpp"

#include <filesystem>
#include <fstream>

std::string FileUtils::read(std::string path) {
    if (!std::filesystem::exists(path)) {
        throw std::invalid_argument("File '" + path + "' not found");
    }
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::system_error(errno, std::generic_category(), "Failed to open '" + path + "'");
    }

    std::string buf;

    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}