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
out vec3 light;
// flat out uint rot;
// flat out uint edges;
// out vec2 outlight;

uniform mat4 MVmat;
uniform mat4 Pmat;
uniform vec3 sundir;
uniform vec3 suncolor;

float voxSize = 1;
const float texSize = 0.5;

// bool blending_light = false;
vec3 curlight;
// float num_lights;
uvec3 outattr;

uint extract_uint4(uint val, uint index) {
	return (val >> (index*4u)) & 0xfu;
}

vec2 get_light(uvec2 data, vec4 normal) {
	float sundot = dot(sundir, normal.xyz/voxSize);
	return vec2(float(extract_uint4(data.y, 0u)) - (sundot*3+3), float(extract_uint4(data.y, 2u))) / 15;
	// return vec2(1,1);
}

void gen_attr(uvec2 data, vec4 normal) {
	// outattr.z = 0u;
	outattr.x = data.x & 0xffffu;
	// outattr.y = (data.y & 0xff00u) >> 8u;
	vec2 lightlevels = get_light(data, normal);
	bool usesun = lightlevels.x > lightlevels.y;
	curlight = float(usesun) * lightlevels.xxx * suncolor + float(!usesun) * lightlevels.yyy;
}

void set_attr() {
	// edges = outattr.z;
	tex = outattr.x;
	// rot = outattr.y;
	light = curlight;
}

bool is_quad_visible(vec4 position, vec4 normal) {
	return dot(position, normal) <= 0;
}

void addQuad(vec4 position, vec4 dy, vec4 dx, vec4 normal, uvec2 data) {
	if (!is_quad_visible(position, normal)) {
		return;
	}
	gen_attr(data, normal);
	
  UV = vec2(voxSize/texSize,0);
	set_attr();
  gl_Position = Pmat * (position + dx - dy);
  EmitVertex();
	
	UV = vec2(0,0);
	set_attr();
	gl_Position = Pmat * (position - dx - dy);
  EmitVertex();

	UV = vec2(voxSize/texSize,voxSize/texSize);
	set_attr();
	gl_Position = Pmat * (position + dx + dy);
  EmitVertex();

	UV = vec2(0,voxSize/texSize);
	set_attr();
  gl_Position = Pmat * (position - dx + dy);
  EmitVertex();

  EndPrimitive();
}

vec4 rotatePos(vec4 v, vec4 q) {
	return vec4(v.xyz + 2.0*cross(q.xyz, cross(q.xyz, v.xyz) + q.w * v.xyz), v.w);
}

void main() {
	if (isnan(scale[0])) {
		return;
	}
	
  vec4 center = MVmat * gl_in[0].gl_Position;
	
	voxSize = scale[0] / 2;
	//vec4 center = (position + MVmat * vec4(voxSize,voxSize,voxSize,0));
	// vec4 rotquat = rotation[0];
	
	vec4 screencenter = Pmat * center;
	float divisor = screencenter.w + voxSize;
	if (center.z > 1.8*voxSize || abs(max(screencenter.x/divisor, screencenter.y/divisor)) > 1 + 1.8*voxSize/divisor) {
		return;
	}
	
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
