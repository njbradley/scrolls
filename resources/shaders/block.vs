#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 position;
layout(location = 1) in float inscale;

layout(location = 2) in uvec2 infacepx;
layout(location = 3) in uvec2 infacepy;
layout(location = 4) in uvec2 infacepz;
layout(location = 5) in uvec2 infacenx;
layout(location = 6) in uvec2 infaceny;
layout(location = 7) in uvec2 infacenz;


flat out uvec2 facepx;
flat out uvec2 facepy;
flat out uvec2 facepz;
flat out uvec2 facenx;
flat out uvec2 faceny;
flat out uvec2 facenz;

flat out float scale;

void main(){
	
	facepx = infacepx;
	facepy = infacepy;
	facepz = infacepz;
	facenx = infacenx;
	faceny = infaceny;
	facenz = infacenz;
	
	scale = inscale;
	
	gl_Position = vec4(position, 1);
}
