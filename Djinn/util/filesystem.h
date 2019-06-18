#pragma once

#include <filesystem>
#include <string>

namespace djinn::util {
    std::string loadTextFile(const std::filesystem::path& p);
}
