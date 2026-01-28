#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <vector>

namespace FileUtils {

constexpr static int DEFAULT_LINE_COUNT = 3;

/**
 * Read a chunk of text in the file
 * @param start_lno The first line to read
 * @param line_count the number of lines to read (truncated if the file ends before read ends)
 * @returns A vector of lines
 * @throws If the position is not in the file (negative or too large)
 */
std::vector<std::string> read_chunk(std::string path, size_t start_pos,
                                    size_t line_count = DEFAULT_LINE_COUNT);

/**
 * Get the string of a file's content
 * @param path The file path
 * @throws If the file does not exist or cannot be opened successfully
 */
std::string read_all(std::string path);
}  // namespace FileUtils

#endif