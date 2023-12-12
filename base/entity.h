#ifndef BASE_ENDTITY_H
#define BASE_ENDTITY_H

#include "common.h"
#include "plugins.h"

#include "blocks.h"

struct Constraint {
    vec3 position;
    vec3 normal;
};

class Entity {
	BASE_PLUGIN(Entity, (istream& ifile));
public:
    FreeNodeView node;
    vec3 velocity;
    float mass;
    vector<Constraint> constraints;

    Entity* globalnext = nullptr;
	
    explicit Entity(FreeNodeView node);
	explicit Entity(istream& ifile);
	virtual ~Entity();

    vec3 get_unconstrained(vec3 dir);
    void calc_constraints();
    void apply_impulse(vec3 impulse);

    virtual void timestep(float curtime, float deltatime);
};

#endif
