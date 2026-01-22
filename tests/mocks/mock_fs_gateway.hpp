#include <filesystem>
#include <string>
#include <unordered_map>

#include "src/io/fs_gateway.hpp"

struct MockFileEntry {
    std::string name;
    std::filesystem::file_time_type last_write_time;
    size_t modification_count;
};

/** Mock FSGateway that works in memory with file modification logs */
class MockFsGateway : public FSGateway {
   public:
    bool exists(std::string filename) const override;

    std::filesystem::file_time_type last_write_time(std::string filename) const override;

    void touch(std::string filename) override;

    void touch_at(std::string filename, std::filesystem::file_time_type time);

   private:
    std::unordered_map<std::string, MockFileEntry> name_to_file;
};
