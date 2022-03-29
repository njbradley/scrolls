#ifndef LIGHTING_H
#define LIGHTING_H

#include "common.h"
#include "plugins.h"

class LightEngine {
	BASE_PLUGIN(LightEngine, ());
public:
	virtual ~LightEngine() {}
	
	virtual bool update(BlockView node) = 0;
};


class DefaultLight : public LightEngine {
	PLUGIN(DefaultLight);
public:
	virtual bool update(BlockView node);
};

#endif
