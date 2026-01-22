#include "mock_fs_gateway.hpp"

#include <chrono>
#include <filesystem>
#include <stdexcept>

bool MockFsGateway::exists(std::string filename) const { return name_to_file.contains(filename); }

std::filesystem::file_time_type MockFsGateway::last_write_time(std::string filename) const {
    auto entry = name_to_file.find(filename);
    if (entry == name_to_file.end()) {
        throw std::invalid_argument("Cannot get last write time of '" + filename + "'");
    }

    return entry->second.last_write_time;
}

void MockFsGateway::touch(std::string filename) {
    touch_at(filename, std::chrono::file_clock::now());
}

void MockFsGateway::touch_at(std::string filename, std::filesystem::file_time_type time) {
    auto entry = name_to_file.find(filename);
    if (entry == name_to_file.end()) {
        name_to_file[filename] = MockFileEntry{filename, time, 0};
    } else {
        name_to_file[filename].last_write_time = std::chrono::file_clock::now();
        name_to_file[filename].write_count++;
    }
}

size_t MockFsGateway::get_write_count(std::string filename) {
    auto entry = name_to_file.find(filename);
    if (entry == name_to_file.end()) {
        throw std::invalid_argument("Cannot get write count of '" + filename + "'");
    }

    return entry->second.write_count;
}
