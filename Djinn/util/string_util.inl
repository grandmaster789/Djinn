#pragma once

#include "string_util.h"
#include <sstream>

namespace djinn::util {
    template <typename...tArgs>
    std::string stringify(tArgs&&...args) {
        std::stringstream sstr;
    
        (sstr << ... << std::forward<tArgs>(args));
    
        return sstr.str();
    }
}