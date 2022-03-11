#include "blockiter.h"

NodeIndex DirNodeIter::increment_func(NodeIndex nodepos) {
	ivec3 pos = nodepos;
	pos.z++;
	if (pos.z > endpos().z()) {
		pos.z = startpos().z();
		pos.y ++;
		if (pos.y > endpos().y()) {
			pos.y = startpos().y();
			pos.x ++;
		}
	}
	return pos;
}



bool FlagNodeIter::valid_tree() {
	bool result = node->flags & flag;
	node->flags &= ~flag;
	return result;
}
