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
	return point.x >= 0 and point.y >= 0 and point.z >= 0
		and point.x <= scale and point.y <= scale and point.z <= scale;
	return point.x >= position.x and point.y >= position.y and point.z >= position.z
				and point.x <= position.x + scale and point.y <= position.y + scale and point.z <= position.z + scale;
}

bool HitCube::collides(vec3 point) const {
	point = transform_in(point);
	return point.x > 0 and point.y > 0 and point.z > 0
		and point.x < scale and point.y < scale and point.z < scale;
}

bool HitCube::collides(const HitCube& globalother) const {
	HitCube other = transform_in(globalother);
	
	float x_proj = 5;
	
	
	
	return false;
}







HitBox HitBox::transform_in(const HitBox& cube) const {
	return HitBox(transform_in(cube.position), cube.dims, glm::inverse(rotation) * cube.rotation);
}

HitBox HitBox::transform_out(const HitBox& cube) const {
	return HitBox(transform_out(cube.position), cube.dims, rotation * cube.rotation);
}

bool HitBox::contains(vec3 point) const {
	point = transform_in(point);
	return point.x >= 0 and point.y >= 0 and point.z >= 0
		and point.x <= dims.x and point.y <= dims.y and point.z <= dims.z;
}

bool HitBox::collides(vec3 point) const {
	point = transform_in(point);
	return point.x > 0 and point.y > 0 and point.z > 0
		and point.x < dims.x and point.y < dims.y and point.z < dims.z;
}

bool HitBox::collides(const HitBox& globalother) const {
	HitBox other = transform_in(globalother);
	//cout << "Finding collision of " << *this << ' ' << globalother << endl;
	//cout << " in my space " << other << endl;

	vec3 midpoint = dims/2.0f;
	vec3 other_midpoint = other.transform_out(other.dims/2.0f);
	vec3 vec_between = other_midpoint - midpoint;
	
	if (glm::length(vec_between) > (glm::length(dims) + glm::length(other.dims))/2.0f) {
		return false;
	}

	if (glm::length(vec_between) < (std::min(dims.x, std::min(dims.y, dims.z))
			+ std::min(other.dims.x, std::min(other.dims.y, other.dims.z))) / 2.0f) {
		//cout << "Shortcut touching" << vec_between << ' ' << dims << ' ' << other.dims << endl;;
		return true;
	}

	vec3 dirx = other.transform_out_dir(vec3(1,0,0));
	vec3 diry = other.transform_out_dir(vec3(0,1,0));
	vec3 dirz = other.transform_out_dir(vec3(0,0,1));

	vector<vec3> axes_to_check {
		{1,0,0},
		{0,1,0},
		{0,0,1}
	};

	if (other.rotation != quat(1,0,0,0)) {
		cout << "adding new axes to check" << endl;
		vec3 new_axes[] = {
			dirx, diry, dirz,
			glm::cross(vec3(1,0,0), dirx),
			glm::cross(vec3(1,0,0), diry),
			glm::cross(vec3(1,0,0), dirz),
			glm::cross(vec3(0,1,0), dirx),
			glm::cross(vec3(0,1,0), diry),
			glm::cross(vec3(0,1,0), dirz),
			glm::cross(vec3(0,0,1), dirx),
			glm::cross(vec3(0,0,1), diry),
			glm::cross(vec3(0,0,1), dirz)
		};

		axes_to_check.insert(axes_to_check.end(), new_axes, new_axes+12);
	}

	for (vec3 axis : axes_to_check) {
		if (glm::length(axis) > 0) {
			axis = axis / glm::length(axis);
			//cout << "  axis " << axis;
			float my_proj = glm::dot(dims/2.0f, glm::abs(axis));
			float other_proj = std::abs(glm::dot(dirx, axis))
			                 + std::abs(glm::dot(diry, axis))
			                 + std::abs(glm::dot(dirz, axis));
			float between_proj = std::abs(glm::dot(vec_between, axis));
			
			float space = between_proj - (my_proj + other_proj) / 2.0f;
			//cout << "   " << my_proj << ' ' << other_proj << ' ' << between_proj << ' ' << space << endl;
			//cout << "     " << dirx << ' ' << diry << ' ' << dirz << ' ' << endl;
			if (space >= 0) {
				return false;
			}
		}
	}

	return true;
}

