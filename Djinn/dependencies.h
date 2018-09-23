#pragma once

// Platform detection macros and platform specific definitions
#include "preprocessor.h"

// http://www.glfw.org
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>

// The C++ interface for Vulkan, SPIRV and shaderC
//#include <vulkan/vulkan.hpp> // this is already included by GLFW
//#include <vulkan/spirv.hpp>
//#include <shaderc/shaderc.hpp>

// GLM optimized floating point linear algebra
// 
#if DJINN_PLATFORM == DJINN_PLATFORM_WINDOWS
    // disable GLM's warnings under MSVC
	#pragma warning(push)
	#pragma warning(disable: 4201) // nonstandard extension used: nameless struct/union
	#pragma warning(disable: 4310) // cast truncates constant value
#endif

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_META_PROG_HELPERS
#define GLM_ENABLE_EXPERIMENTAL
	#include <glm/glm.hpp>
	#include <glm/gtc/matrix_transform.hpp>
	#include <glm/gtc/quaternion.hpp>
	#include <glm/gtc/type_ptr.hpp>
	#include <glm/gtx/transform2.hpp>
	#include <glm/ext.hpp>
#undef GLM_ENABLE_EXPERIMENTAL
#undef GLM_META_PROG_HELPERS
#undef GLM_FORCE_DEPTH_ZERO_TO_ONE
#undef GLM_FORCE_RADIANS

// additional ostream operators for glm
namespace glm {
	std::ostream& operator << (std::ostream& os, const vec2& v);
	std::ostream& operator << (std::ostream& os, const vec3& v);
	std::ostream& operator << (std::ostream& os, const vec4& v);

	std::ostream& operator << (std::ostream& os, const mat2& m);
	std::ostream& operator << (std::ostream& os, const mat3& m);
	std::ostream& operator << (std::ostream& os, const mat4& m);
}

#if DJINN_PLATFORM == DJINN_PLATFORM_WINDOWS
	#pragma warning(pop)
#endif

// JSON interoperability
#include <json.hpp> // https://github.com/nlohmann/json
