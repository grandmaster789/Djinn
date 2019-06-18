#include "filesystem.h"
#include <fstream>

namespace djinn::util {
    std::string loadTextFile(const std::filesystem::path& p) {
        using namespace std::filesystem;

        if (!exists(p))
            throw std::runtime_error("Path does not exist");

        if (!is_regular_file(p))
            throw std::runtime_error("Path is not a regular file");

        std::ifstream in(p.string());

        if (!in.good())
            throw std::runtime_error("Failed to open file");

        in.seekg(0, std::ios::end);
        size_t filesize = in.tellg();
        in.seekg(0);

        std::string result;
        result.resize(filesize);
        in.read(result.data(), filesize);

        return result;
    }
}  // namespace djinn::util
