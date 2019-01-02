#pragma once

// Platform detection macros and platform specific definitions
#include "preprocessor.h"

// The C++ interface for Vulkan, SPIRV and shaderC
#include <vulkan/vulkan.hpp> // https://vulkan.lunarg.com/doc/sdk/1.1.92.1/windows/vkspec.html
#include <vulkan/spirv.hpp>
#include <shaderc/shaderc.hpp>

// ~~ make sure to link with vulkan-1.lib in the application project...
// ~~ please follow https://vulkan.lunarg.com/doc/sdk/1.1.92.1/windows/spirv_toolchain.html to obtain debug-compatible shaderc library builds
#if DJINN_PLATFORM == DJINN_PLATFORM_WINDOWS
    #pragma comment(lib, "vulkan-1.lib")         // this #pragma is really more MSVC specific, but close enough

    #ifdef DJINN_DEBUG
        #pragma comment(lib, "shaderc_combinedd.lib") // unfortunately the shaderc build uses the same name for debug and release builds; manual renaming seems easiest
    #else
        #pragma comment(lib, "shaderc_combined.lib") // use the shaderc toolchain for integrated spirv compilation of shaders
    #endif
#endif

// JSON interoperability
#include <json.hpp> // https://github.com/nlohmann/json
