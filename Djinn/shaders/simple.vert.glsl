#version 450

#extension GL_ARB_separate_shader_objects: enable
#extension GL_ARB_shading_language_420pack: enable

// assuming vertex format is     { float3 position; float3 color}
// assuming bound uniform buffer { mat4 projection; mat4 model; mat4 view }

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

layout (binding = 0) uniform UBO {
	mat4 m_Projection;
	mat4 m_Model;
	mat4 m_View;
} ubo;

layout (location = 0) out vec3 outColor;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	outColor = inColor;

	gl_Position =
		ubo.m_Projection *
		ubo.m_View       *
		ubo.m_Model      * 
		vec4(inPos.xyz, 1.0);
}