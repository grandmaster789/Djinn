#version 450

#extension GL_ARB_separate_shader_objects: enable
#extension GL_ARB_shading_language_420pack: enable

// assuming vertex format is     { float3 position; float3 color}
// assuming bound uniform buffer { mat4 projection; mat4 model; mat4 view }

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec3 inColor;

layout (binding = 0) uniform UBO {
	mat4 m_Model;
	mat4 m_View;
	mat4 m_Projection;	
} ubo;

layout (location = 0) out vec4 outColor;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	outColor = vec4(inColor, 1.0);

	gl_Position =
		ubo.m_Projection *
		ubo.m_View       *
		ubo.m_Model      * 
		vec4(inPos.xyz, 1.0); // this allows us to switch to vec3 input at any given time

	// alternatively (the slightly more traditional MVP variant)
	/*
	gl_Position =
		inpos       *
		ubo.m_Model *
		ubo.m_View  * 
		ubo.m_Projection;
	*/
}