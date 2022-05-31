#ifndef BASE_ENTITY
#define BASE_ENTITY

#include "common.h"


// Class that represents an axis alligned,
// integer alligned, cube hitbox.
struct IHitCube {
	ivec3 position;
	int scale;
	
	IHitCube(ivec3 pos, int nscale);
	
	// A point is contained if it is inside the box or on the edge
	bool contains(ivec3 point) const;
	// a box is contained when no part of it extends outside the box
	// (so the box can also touch the edges)
	bool contains(const IHitCube& other) const;
	
	// A point or box collides when it actually penetrates the box, a point
	// is not colliding if it is on the edge
	bool collides(ivec3 point) const;
	bool collides(const IHitCube& other) const;
};







///// INLINE FUNCTIONS

inline IHitCube::IHitCube(ivec3 pos, int nscale): position(pos), scale(nscale) {
	
}

inline bool IHitCube::contains(ivec3 point) const {
	return point.x >= position.x and point.y >= position.y and point.z >= position.z
			and point.x <= position.x + scale and point.y <= position.y + scale and point.z <= position.z + scale;
}

inline bool IHitCube::contains(const IHitCube& other) const {
	return other.position.x >= position.x and other.position.y >= position.y and other.position.z >= position.z
			and other.position.x + other.scale <= position.x + scale and other.position.y + other.scale <= position.y + scale
			and other.position.z + other.scale <= position.z + scale;
}

inline bool IHitCube::collides(ivec3 point) const {
	return point.x > position.x and point.y > position.y and point.z > position.z
			and point.x < position.x + scale and point.y < position.y + scale and point.z < position.z + scale;
}

inline bool IHitCube::collides(const IHitCube& other) const {
	return (position.x <= other.position.x and position.x + scale > other.position.x)
			or (other.position.x <= position.x and other.position.x + other.scale > position.x)
			or (position.y <= other.position.y and position.y + scale > other.position.y)
			or (other.position.y <= position.y and other.position.y + other.scale > position.y)
			or (position.z <= other.position.z and position.z + scale > other.position.z)
			or (other.position.z <= position.z and other.position.z + other.scale > position.z);
}

	

#endif
