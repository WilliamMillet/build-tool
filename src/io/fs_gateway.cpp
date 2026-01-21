#include "fs_gateway.hpp"

#include <filesystem>
#include <fstream>

#include "../errors/error.hpp"

bool ProdFSGateway::exists(std::string filename) const try {
    return std::filesystem::exists(filename);
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Determining file '" + filename + "' existence");
}

std::filesystem::file_time_type ProdFSGateway::last_write_time(std::string filename) const try {
    return std::filesystem::last_write_time(filename);
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Checking last write time of file '" + filename + "'");
}

void ProdFSGateway::touch(std::string filename) try {
    std::ofstream file(filename);
    if (!file) {
        throw IOError("Failed to create file '" + filename + "'");
    }
} catch (std::exception& excep) {
    Error::update_and_throw(excep, "Scanning for cyclical dependency in rules");
}
