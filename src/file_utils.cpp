#include "file_utils.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

std::vector<std::string> FileUtils::read_chunk(std::string path, size_t start_pos,
                                               size_t line_count) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::system_error(errno, std::generic_category(), "Failed to open '" + path + "'");
    }

    if (start_pos >= fs::file_size(path)) {
        throw std::invalid_argument("Cannot read from position " + std::to_string(start_pos) +
                                    " (0 indexed) as file is only " +
                                    std::to_string(fs::file_size(path)) + " bytes long");
    }

    file.seekg(start_pos);
    std::vector<std::string> lines;
    std::string curr_line;
    while (lines.size() < line_count && std::getline(file, curr_line)) {
        lines.push_back(std::move(curr_line));
    }

    return lines;
}

std::string FileUtils::read_all(std::string path) {
    if (!fs::exists(path)) {
        throw std::invalid_argument("File '" + path + "' not found");
    }
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::system_error(errno, std::generic_category(), "Failed to open '" + path + "'");
    }

    std::string buf;

    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}
