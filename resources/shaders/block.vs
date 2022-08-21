#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv_in;
layout(location = 2) in ivec3 data_in;

out vec2 uv;
out vec2 light;
flat out int tex;

uniform mat4 MVP;
uniform vec3 lightdir;

void main(){
	gl_Position = MVP *  vec4(position, 1);
	uv = uv_in;
	light = vec2(data_in.xy)/255.0f;
	tex = data_in.z;
}
