#version 330 core

in vec2 UV;
flat in uint tex;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2DArray textures;

void main() {
	vec4 tex_color = texture(textures, vec3(UV.x, UV.y, tex-1u));
	if (tex_color.a < 0.1) {
		discard;
	}
	color = tex_color;
}
