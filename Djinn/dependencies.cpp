#include "dependencies.h"

// additional ostream operators for glm
namespace glm {
	std::ostream& operator << (std::ostream& os, const vec2& v) {
		os << to_string(v);
		return os;
	}

	std::ostream& operator << (std::ostream& os, const vec3& v) {
		os << to_string(v);
		return os;
	}

	std::ostream& operator << (std::ostream& os, const vec4& v) {
		os << to_string(v);
		return os;
	}

	std::ostream& operator << (std::ostream& os, const mat2& m) {
		os << to_string(m);
		return os;
	}

	std::ostream& operator << (std::ostream& os, const mat3& m) {
		os << to_string(m);
		return os;
	}

	std::ostream& operator << (std::ostream& os, const mat4& m) {
		os << to_string(m);
		return os;
	}
}