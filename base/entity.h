#ifndef BASE_ENTITY
#define BASE_ENTITY

#include "common.h"


// Class that represents an axis alligned,
// integer alligned, cube hitbox.
struct IHitCube {
	ivec3 position;
	int scale;
	
	IHitCube();
	IHitCube(ivec3 pos, int nscale);
	
	ivec3 midpoint() const;
	
	// A point is contained if it is inside the box or on the edge
	bool contains(ivec3 point) const;
	// a box is contained when no part of it extends outside the box
	// (so the box can also touch the edges)
	bool contains(const IHitCube& other) const;
	
	// A point or box collides when it actually penetrates the box, a point
	// is not colliding if it is on the edge
	bool collides(ivec3 point) const;
	bool collides(const IHitCube& other) const;
	
	ivec3 transform_in(ivec3 point) const;
	vec3 transform_in(vec3 point) const;
	ivec3 transform_out(ivec3 point) const;
	vec3 transform_out(vec3 point) const;
	IHitCube transform_in(const IHitCube& cube) const;
	IHitCube transform_out(const IHitCube& cube) const;
	
	friend IHitCube operator+(const IHitCube& cube, ivec3 pos);
	friend IHitCube operator+(ivec3 pos, const IHitCube& cube);
	IHitCube& operator+=(ivec3 pos);
};


struct HitCube {
	vec3 position;
	int scale;
	quat rotation;
	
	HitCube();
	HitCube(const IHitCube& icube);
	HitCube(vec3 pos, int nscale, quat rot);
	
	bool contains(vec3 point) const;
	bool contains(const HitCube& other) const;
	
	bool collides(vec3 point) const;
	bool collides(const HitCube& other) const;
	
	vec3 transform_in(vec3 point) const;
	vec3 transform_out(vec3 point) const;
	HitCube transform_in(const HitCube& cube) const;
	HitCube transform_out(const HitCube& cube) const;
};






///// INLINE FUNCTIONS

inline IHitCube::IHitCube() {
	
}

inline IHitCube::IHitCube(ivec3 pos, int nscale): position(pos), scale(nscale) {
	
}

inline ivec3 IHitCube::midpoint() const {
	return position + scale/2;
}

inline ivec3 IHitCube::transform_in(ivec3 point) const {
	return point - position;
}

inline vec3 IHitCube::transform_in(vec3 point) const {
	return point - vec3(position);
}

inline ivec3 IHitCube::transform_out(ivec3 point) const {
	return point + position;
}

inline vec3 IHitCube::transform_out(vec3 point) const {
	return point + vec3(position);
}



inline IHitCube operator+(const IHitCube& cube, ivec3 pos) {
	return IHitCube(cube.position + pos * cube.scale, cube.scale);
}

inline IHitCube operator+(ivec3 pos, const IHitCube& cube) {
	return IHitCube(cube.position + pos * cube.scale, cube.scale);
}

inline IHitCube& IHitCube::operator+=(ivec3 pos) {
	position += pos * scale;
	return *this;
}



inline HitCube::HitCube() {
	
}

inline HitCube::HitCube(const IHitCube& icube): position(icube.position), scale(icube.scale), rotation(1,0,0,0) {
	
}

inline HitCube::HitCube(vec3 pos, int nscale, quat rot): position(pos), scale(nscale), rotation(rot) {
	
}

inline vec3 HitCube::transform_in(vec3 point) const {
	return glm::inverse(rotation) * (point - position);
}

inline vec3 HitCube::transform_out(vec3 point) const {
	return rotation * point + position;
}

#endif
