#ifndef BASE_PHYSICS
#define BASE_PHYSICS

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

	friend ostream& operator<<(ostream& out, const IHitCube& cube);
};


struct HitCube {
	vec3 position;
	int scale;
	//quat rotation;
	
	HitCube();
	HitCube(const IHitCube& icube);
	HitCube(vec3 pos, int nscale);//, quat rot);
	
	vec3 midpoint() const;

	bool contains(vec3 point) const;
	bool contains(const HitCube& other) const;
	
	bool collides(vec3 point) const;
	bool collides(const HitCube& other) const;
	
	vec3 transform_in(vec3 point) const;
	vec3 transform_out(vec3 point) const;
	vec3 transform_in_dir(vec3 point) const;
	vec3 transform_out_dir(vec3 point) const;
	HitCube transform_in(const HitCube& cube) const;
	HitCube transform_out(const HitCube& cube) const;

	friend HitCube operator+(const HitCube& cube, ivec3 pos);
	friend HitCube operator+(ivec3 pos, const HitCube& cube);
	HitCube& operator+=(ivec3 pos);

	friend ostream& operator<<(ostream& out, const HitCube& cube);
};

struct HitBox {
	vec3 position;
	vec3 dims;
	//quat rotation;

	HitBox();
	HitBox(const IHitCube& icube);
	HitBox(const HitCube& cube);
	HitBox(vec3 pos, vec3 dims);//, quat rot);

	vec3 midpoint() const;

	bool contains(vec3 point) const;
	bool contains(const HitBox& other) const;

	bool collides(vec3 point) const;
	bool collides(const HitBox& other) const;

	vec3 transform_in(vec3 point) const;
	vec3 transform_out(vec3 point) const;
	vec3 transform_in_dir(vec3 point) const;
	vec3 transform_out_dir(vec3 point) const;
	HitBox transform_in(const HitBox& box) const;
	HitBox transform_out(const HitBox& box) const;

	friend ostream& operator<<(ostream& out, const HitBox& cube);
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

inline ostream& operator<<(ostream& out, const IHitCube& cube) {
	return out << "IHitCube(" << cube.position << ' ' << cube.scale << ")";
}



inline HitCube::HitCube() {
	
}

inline HitCube::HitCube(const IHitCube& icube): position(icube.position), scale(icube.scale) {
	
}

inline HitCube::HitCube(vec3 pos, int nscale): position(pos), scale(nscale) {
	
}

inline vec3 HitCube::transform_in(vec3 point) const {
	//return glm::inverse(rotation) * (point - position);
	return point - position;
}

inline vec3 HitCube::transform_in_dir(vec3 point) const {
	//return glm::inverse(rotation) * point;
    return point;
}

inline vec3 HitCube::transform_out(vec3 point) const {
	//return rotation * point + position;
    return point + position;
}

inline vec3 HitCube::transform_out_dir(vec3 point) const {
	//return rotation * point;
    return point;
}

inline HitCube operator+(const HitCube& cube, ivec3 pos) {
	return HitCube(cube.position + cube.transform_out_dir(pos * cube.scale), cube.scale);
}

inline HitCube operator+(ivec3 pos, const HitCube& cube) {
	return HitCube(cube.position + cube.transform_out_dir(pos * cube.scale), cube.scale);
}

inline HitCube& HitCube::operator+=(ivec3 pos) {
	position += transform_out_dir(pos * scale);
	return *this;
}

inline ostream& operator<<(ostream& out, const HitCube& cube) {
	return out << "HitCube(" << cube.position << ' ' << cube.scale << ")";
}







inline HitBox::HitBox() {
	
}

inline HitBox::HitBox(const IHitCube& ibox): position(ibox.position), dims(ibox.scale) {
	
}

inline HitBox::HitBox(const HitCube& box): position(box.position), dims(box.scale) {
	
}

inline HitBox::HitBox(vec3 pos, vec3 dims): position(pos), dims(dims) {
	
}

inline vec3 HitBox::transform_in(vec3 point) const {
	//return glm::inverse(rotation) * (point - position);
	return point - position;
}

inline vec3 HitBox::transform_in_dir(vec3 point) const {
	//return glm::inverse(rotation) * point;
    return point;
}

inline vec3 HitBox::transform_out(vec3 point) const {
	//return rotation * point + position;
    return point + position;
}

inline vec3 HitBox::transform_out_dir(vec3 point) const {
	//return rotation * point;
    return point;
}

inline ostream& operator<<(ostream& out, const HitBox& cube) {
	return out << "HitBox(" << cube.position << ' ' << cube.dims << ")";
}

#endif
