#include <filesystem>
#include <string>

class FSGateway {
   public:
    ~FSGateway() = default;

    virtual bool exists(std::string filename) const;

    virtual std::filesystem::file_time_type last_write_time(std::string filename) const;

    virtual void touch(std::string filename);
};

/** FS Gateway to use in production that interacts with the real filesystem */
class ProdFSGateway : public FSGateway {
    bool exists(std::string filename) const override;

    std::filesystem::file_time_type last_write_time(std::string filename) const override;

    void touch(std::string filename) override;
};