#include "lighting.h"

#include "blocks.h"
#include "blockiter.h"

DEFINE_PLUGIN(LightEngine);

bool DefaultLight::update(NodeView view) {
	for (NodeView node : BlockIterable<DirBlockIter>(view, ivec3(0,1,0))) {
		while (!node.continues() and node.block()->value == 0) {
			node.block()->sunlight = 15;
			if (!node.moveto(node.globalpos + ivec3(0,-1,0) * node.scale, node.scale)) {
				break;
			}
		}
	}
	
	return true;
}

EXPORT_PLUGIN(DefaultLight);
