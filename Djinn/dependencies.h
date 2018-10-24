#pragma once

// Platform detection macros and platform specific definitions
#include "preprocessor.h"

// The C++ interface for Vulkan, SPIRV and shaderC
#include <vulkan/vulkan.hpp> // GLFW includes the C interface, I want the CPP interface
//#include <vulkan/spirv.hpp>
//#include <shaderc/shaderc.hpp>

// ~~ make sure to link with vulkan-1.lib in the application project...

// JSON interoperability
#include <json.hpp> // https://github.com/nlohmann/json
