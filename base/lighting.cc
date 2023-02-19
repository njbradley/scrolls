#include "lighting.h"

#include "blocks.h"
#include "blockiter.h"

DEFINE_PLUGIN(LightEngine);

bool DefaultLight::update(NodeView view) {
	for (NodeView node : NodeIterable<DirBlockIter<NodeView>>(view, ivec3(0,1,0))) {
		while (!node.haschildren() and node.block()->type == nullptr) {
			node.block()->sunlight = 15;
			// if (!node.moveto(node.position + ivec3(0,-1,0) * node.scale, node.scale)) {
			// 	break;
			// }
		}
	}
	
	return true;
}

EXPORT_PLUGIN(DefaultLight);
