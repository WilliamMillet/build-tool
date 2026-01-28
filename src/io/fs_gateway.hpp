#ifndef FS_GATEWAY_H
#define FS_GATEWAY_H

#include <filesystem>
#include <string>

/** Interface between program and file system. Primarily useful for enabling dependency injection */
class FSGateway {
   public:
    virtual ~FSGateway() = default;

    virtual bool exists(std::string filename) const = 0;

    virtual std::filesystem::file_time_type last_write_time(std::string filename) const = 0;

    /**
     * Create a file
     * @param filename The filename of the new file
     * @throws If the file cannot be created successfully
     */
    virtual void touch(std::string filename) = 0;
};

/** FS Gateway to use in production that interacts with the real filesystem */
class ProdFSGateway : public FSGateway {
    bool exists(std::string filename) const override;

    std::filesystem::file_time_type last_write_time(std::string filename) const override;

    void touch(std::string filename) override;
};

#endif
