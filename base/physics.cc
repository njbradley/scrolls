#include "physics.h"


bool IHitCube::contains(ivec3 point) const {
	return point.x >= position.x and point.y >= position.y and point.z >= position.z
			and point.x <= position.x + scale and point.y <= position.y + scale and point.z <= position.z + scale;
}

bool IHitCube::contains(const IHitCube& other) const {
	return other.position.x >= position.x and other.position.y >= position.y and other.position.z >= position.z
			and other.position.x + other.scale <= position.x + scale and other.position.y + other.scale <= position.y + scale
			and other.position.z + other.scale <= position.z + scale;
}

bool IHitCube::collides(ivec3 point) const {
	return point.x > position.x and point.y > position.y and point.z > position.z
			and point.x < position.x + scale and point.y < position.y + scale and point.z < position.z + scale;
}

bool IHitCube::collides(const IHitCube& other) const {
	return ((position.x <= other.position.x and position.x + scale > other.position.x)
			or (other.position.x <= position.x and other.position.x + other.scale > position.x))
			and ((position.y <= other.position.y and position.y + scale > other.position.y)
			or (other.position.y <= position.y and other.position.y + other.scale > position.y))
			and ((position.z <= other.position.z and position.z + scale > other.position.z)
			or (other.position.z <= position.z and other.position.z + other.scale > position.z));
}



HitCube HitCube::transform_in(const HitCube& cube) const {
	return HitCube(transform_in(cube.position), cube.scale, glm::inverse(rotation) * cube.rotation);
}

HitCube HitCube::transform_out(const HitCube& cube) const {
	return HitCube(transform_out(cube.position), cube.scale, rotation * cube.rotation);
}

bool HitCube::contains(vec3 point) const {
	point = transform_in(point);
	return point.x >= position.x and point.y >= position.y and point.z >= position.z
				and point.x <= position.x + scale and point.y <= position.y + scale and point.z <= position.z + scale;
}

bool HitCube::collides(vec3 point) const {
	point = transform_in(point);
	return point.x > position.x and point.y > position.y and point.z > position.z
			and point.x < position.x + scale and point.y < position.y + scale and point.z < position.z + scale;
}

bool HitCube::collides(const HitCube& globalother) const {
	HitCube other = transform_in(globalother);
	
	float x_proj = 5;
	
	
	
	return false;
}
