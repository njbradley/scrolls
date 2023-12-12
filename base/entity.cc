#include "entity.h"

Entity::Entity(FreeNodeView node): node(node), velocity(0,0,0) {

}

Entity::Entity(istream& ifile) {

}

Entity::~Entity() {
    
}

void Entity::calc_constraints() {
    for (Direction dir : Direction::all) {
        
    }
}

vec3 Entity::get_unconstrained(vec3 dir) {
    vec3 normdir = glm::normalize(dir);
    for (const Constraint& constraint : constraints) {
        float projection = glm::dot(constraint.normal, normdir);
        if (projection < 0) {
            dir -= normdir * projection;
        }
    }
    return dir;
}

void Entity::apply_impulse(vec3 impulse) {
    velocity += get_unconstrained(impulse) / mass;
}

void Entity::timestep(float curtime, float deltatime) {
    velocity += vec3(0,-10,0) * deltatime;

    velocity = get_unconstrained(velocity);
    node.position += velocity * deltatime;


    //node.recalculate_position();
}

DEFINE_PLUGIN(Entity);
