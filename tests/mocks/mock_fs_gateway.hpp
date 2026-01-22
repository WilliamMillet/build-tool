#ifndef MOCK_FS_GATEWAY_H
#define MOCK_FS_GATEWAY_H

#include <filesystem>
#include <string>
#include <unordered_map>

#include "src/io/fs_gateway.hpp"

struct MockFileEntry {
    std::string name;
    std::filesystem::file_time_type last_write_time;
    size_t write_count;
};

/** Mock FSGateway that works in memory with file modification logs */
class MockFsGateway : public FSGateway {
   public:
    bool exists(std::string filename) const override;

    std::filesystem::file_time_type last_write_time(std::string filename) const override;

    void touch(std::string filename) override;

    /** Mock the creation of a file at a specified time */
    void touch_at(std::string filename, std::filesystem::file_time_type time);

    /** Get the number of times a file has been written to */
    size_t get_write_count(std::string filename);

   private:
    std::unordered_map<std::string, MockFileEntry> name_to_file;
};

#endif