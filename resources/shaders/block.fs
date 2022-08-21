#version 330 core

in vec2 uv;
flat in int tex;
in vec2 light;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2DArray textures;
uniform vec3 suncolor;

void main() {
	vec3 light_rgb = light.x * suncolor + light.y;
	vec4 tex_color = texture(textures, vec3(uv.x, uv.y, tex-1)) * vec4(light_rgb, 1);
	if (tex_color.a < 0.1) {
		discard;
	}
	color = tex_color;
}
