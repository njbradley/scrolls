#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

flat in uvec2 facepx[];
flat in uvec2 facepy[];
flat in uvec2 facepz[];
flat in uvec2 facenx[];
flat in uvec2 faceny[];
flat in uvec2 facenz[];

flat in float scale[];

out vec2 UV;
flat out uint tex;
// flat out uint rot;
// flat out uint edges;
// out vec2 outlight;

uniform mat4 MVmat;
uniform mat4 Pmat;
// uniform vec3 sunlight;

float voxSize = 1;

bool blending_light = false;
vec2 light;
float num_lights;
uvec3 outattr;

vec2 get_light(uvec2 data, vec4 normal) {
	// float sundot = -dot(sunlight, normal.xyz/voxSize);
	// return vec2(float((data.y & 0xff000000u) >> 24u) - (sundot*5+5), float((data.y & 0xff0000u) >> 16u)) / 20;
	return vec2(1,1);
}

void gen_attr(uvec2 data, vec4 normal) {
	// outattr.z = 0u;
	outattr.x = data.x;
	// outattr.y = (data.y & 0xff00u) >> 8u;
	// light = get_light(data, normal);
}

void set_attr() {
	// edges = outattr.z;
	tex = outattr.x;
	// rot = outattr.y;
	// outlight = light;
}

bool is_quad_visible(vec4 position, vec4 normal) {
	return dot(position, normal) <= 0;
}

void addQuad(vec4 position, vec4 dy, vec4 dx, vec4 normal, uvec2 data) {
	if (false && !is_quad_visible(position, normal)) {
		UV = vec2(0,0);
		tex = 1u;
		gl_Position = vec4(0,0,0,1);
		EmitVertex();
		gl_Position = vec4(1,0,0,1);
		EmitVertex();
		gl_Position = vec4(1,1,0,1);
		EmitVertex();
		return;
	}
	gen_attr(data, normal);
	
  UV = vec2(1,0);
	set_attr();
  gl_Position = Pmat * (position + dx - dy);
  EmitVertex();
	
	UV = vec2(0,0);
	set_attr();
	gl_Position = Pmat * (position - dx - dy);
  EmitVertex();

	UV = vec2(1,1);
	set_attr();
	gl_Position = Pmat * (position + dx + dy);
  EmitVertex();

	UV = vec2(0,1);
	set_attr();
  gl_Position = Pmat * (position - dx + dy);
  EmitVertex();

  EndPrimitive();
}

vec4 rotatePos(vec4 v, vec4 q) {
	return vec4(v.xyz + 2.0*cross(q.xyz, cross(q.xyz, v.xyz) + q.w * v.xyz), v.w);
}

void main() {
	
	// if (scale[0] < 3) {
	// 	UV = vec2(0,0);
	// 	tex = 1u;
	// 	gl_Position = vec4(0,0,0,1);
	// 	EmitVertex();
	// 	gl_Position = vec4(1,0,0,1);
	// 	EmitVertex();
	// 	gl_Position = vec4(1,1,0,1);
	// 	EmitVertex();
	// }
	// return;
	
	if (false) { //isnan(scale[0])) {
		
		UV = vec2(0,0);
		tex = 0u;
		gl_Position = vec4(0,0,0,1);
		EmitVertex();
		gl_Position = vec4(1,0,0,1);
		EmitVertex();
		gl_Position = vec4(1,1,0,1);
		EmitVertex();
		
		return;
	}
	
  vec4 center = MVmat * gl_in[0].gl_Position;
	
	voxSize = scale[0] / 2;
	//vec4 center = (position + MVmat * vec4(voxSize,voxSize,voxSize,0));
	// vec4 rotquat = rotation[0];
	
	vec4 screencenter = Pmat * center;
	float divisor = screencenter.w + voxSize;
	if (false) {//center.z > 1.8*voxSize || abs(max(screencenter.x/divisor, screencenter.y/divisor)) > 1 + 1.8*voxSize/divisor) {
		
		UV = vec2(0,0);
		tex = 1u;
		gl_Position = vec4(0,0,0,1);
		EmitVertex();
		gl_Position = vec4(-1,0,0,1);
		EmitVertex();
		gl_Position = vec4(-1,1,0,1);
		EmitVertex();
		
		return;
	}
	
	
  // vec4 dx = vec4(1,0,0,0);
  // vec4 dy = vec4(0,1,0,0);
  // vec4 dz = vec4(0,0,1,0);
	
  vec4 dx = (MVmat[0] * voxSize);
  vec4 dy = (MVmat[1] * voxSize);
  vec4 dz = (MVmat[2] * voxSize);
	
  // vec4 dx = MVmat * rotatePos(vec4(voxSize,0,0,0), rotquat);
  // vec4 dy = MVmat * rotatePos(vec4(0,voxSize,0,0), rotquat);
  // vec4 dz = MVmat * rotatePos(vec4(0,0,voxSize,0), rotquat);
	
	if ((facepx[0].x) != 0u) {
  	addQuad(center + dx, dy, dz,  dx, facepx[0]);
  }
  if ((facenx[0].x) != 0u) {
    addQuad(center - dx, dz, dy,  -dx, facenx[0]);
	}
  if ((facepy[0].x) != 0u) {
    addQuad(center + dy, dz, dx,  dy, facepy[0]);
	}
  if ((faceny[0].x) != 0u) {
    addQuad(center - dy, dx, dz,  -dy, faceny[0]);
	}
  if ((facepz[0].x) != 0u) {
    addQuad(center + dz, dx, dy,  dz, facepz[0]);
	}
  if ((facenz[0].x) != 0u) {
    addQuad(center - dz, dy, dx,  -dz, facenz[0]);
	}
}
