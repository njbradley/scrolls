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
	
	friend IHitCube operator+(const IHitCube& cube, ivec3 pos);
	friend IHitCube operator+(ivec3 pos, const IHitCube& cube);
	IHitCube& operator+=(ivec3 pos);
};


struct HitCube {
	vec3 position;
	float scale;
	quat rotation;
	
	HitCube();
	HitCube(vec3 pos, int nscale, quat rot);
	
	bool contains(vec3 point) const;
	bool contains(const HitCube& other) const;
	
	bool collides(vec3 point) const;
	bool collides(const HitCube& other) const;
};






///// INLINE FUNCTIONS

inline IHitCube::IHitCube() {
	
}

inline IHitCube::IHitCube(ivec3 pos, int nscale): position(pos), scale(nscale) {
	
}

inline ivec3 IHitCube::midpoint() const {
	return position + scale/2;
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

	

#endif
