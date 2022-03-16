#version 330 core

in vec2 UV;
flat in uint tex;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.

void main() {
	color = vec4(UV.x, UV.y, tex, 1);
}
