#pragma once

#include "string_util.h"
#include <sstream>

namespace djinn::util {
    template <typename...tArgs>
    std::string stringify(tArgs&&...args) {
        std::stringstream sstr;
    
        (sstr << ... << std::forward<tArgs>(args)); // C++17 fold expression
    
        // for C++11 you can use an initializer list to achieve the same:
        /*(void)std::initializer_list<int> {
            (sstr << std::forward<tArgs>(args), 0)...
        };*/
    
        return sstr.str();
    }
}