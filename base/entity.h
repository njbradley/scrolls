#ifndef BASE_ENDTITY_H
#define BASE_ENDTITY_H

#include "common.h"
#include "plugins.h"

class Entity {
	BASE_PLUGIN(Entity, (istream& ifile));
public:
	
	Entity(istream& ifile);
	virtual ~Entity();
};

#endif
