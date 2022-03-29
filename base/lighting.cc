#include "lighting.h"

#include "arrblocks.h"
#include "arrblockiter.h"

DEFINE_PLUGIN(LightEngine);

bool DefaultLight::update(BlockView view) {
	return true;
}

EXPORT_PLUGIN(DefaultLight);
