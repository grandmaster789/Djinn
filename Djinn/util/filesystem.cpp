#include "filesystem.h"
#include <fstream>
#include <sstream>

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

        /*
        // this works, but is slow for large files
        return std::string(
            (std::istreambuf_iterator<char>(in)),
            (std::istreambuf_iterator<char>()));
            */

        std::stringstream buffer;
        buffer << in.rdbuf();

        return buffer.str();
    }
}  // namespace djinn::util
