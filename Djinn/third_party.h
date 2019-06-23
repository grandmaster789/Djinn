#pragma once

// Platform detection macros and platform specific definitions
#include "preprocessor.h"

// The C++ interface for Vulkan, SPIRV and shaderC
#include <shaderc/shaderc.hpp>
#include <vulkan/spirv.hpp>
#include <vulkan/vulkan.hpp>  // https://vulkan.lunarg.com/doc/sdk/1.1.92.1/windows/vkspec.html

#include "vk_ostream.h"

#pragma warning(push)
#define GLM_FORCE_RADIANS

#pragma warning(disable : 4201)  // nonstandard extension used: nameless struct/union
#pragma warning(disable : 4127)  // conditional expression is constant
#pragma warning(disable : 4310)  // cast truncates constant value

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#undef GLM_FORCE_RADIANS
#pragma warning(pop)

// ~~ make sure to link with vulkan-1.lib in the application project...
// ~~ please follow https://vulkan.lunarg.com/doc/sdk/1.1.106.0/windows/spirv_toolchain.html to obtain debug-compatible
// shaderc library builds

#if DJINN_PLATFORM == DJINN_PLATFORM_WINDOWS
// this #pragma is really more MSVC specific, but close enough
#pragma comment(lib, "vulkan-1.lib")

// use the shaderc toolchain for integrated spirv compilation of shaders
#ifdef DJINN_DEBUG
#pragma comment(lib, "shaderc_combinedd.lib")  // manually build the shaderc library, gave it another name
#else
#pragma comment(lib, "shaderc_combined.lib")
#endif
#endif

// JSON interoperability
#include <json.hpp>  // https://github.com/nlohmann/json
