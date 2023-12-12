#include "blockiter.h"

template <typename NodePtrT>
NodeIter<NodePtrT>::NodeIter(const NodePtrT& nnode): node(nnode), highest_node(nnode) {

}


template <typename NodePtrT>
void NodeIter<NodePtrT>::step_down() {
	// cout << "step down " << position << ' ' << scale << endl;
	if (node.haschildren()) {
		node = node.child(startpos());
		// node.step_down(startpos());
		get_safe();
	} else if (node.hasfreechild()) {
		node = node.freechild();
		get_safe();
	} else {
		step_side();
	}
}

template <typename NodePtrT>
void NodeIter<NodePtrT>::step_side() {
	// cout << "step side " << position << ' ' << scale << endl;
	if (!node.hasparent() or node == highest_node) {
		finish();
	} else if (node.isfreenode()) {
		if (node.hasfreesibling()) {
			node = node.freesibling();
			get_safe();
		} else {
			node = node.parent();
			step_side();
		}
	} else if (node.parentindex() == endpos()) {
		node = node.parent();
		if (node.hasfreechild()) {
			node = node.freechild();
			get_safe();
		} else {
			step_side();
		}
	} else {
		// node.step_side(increment_func(node.parentindex()));
		node = node.sibling(increment_func(node.parentindex()));
		get_safe();
	}
}

template <typename NodePtrT>
void NodeIter<NodePtrT>::get_safe() {
	// cout << "get safe " << position << ' ' << scale << endl;
	if (!valid_tree()) {
		// cout << "invalid tree" << endl;
		step_side();
	} else if (!valid_node()) {
		// cout << "invalid node" << endl;
		step_down();
	}
}

template <typename NodePtrT>
NodeIter<NodePtrT> NodeIter<NodePtrT>::operator++() {
	if (node.isvalid()) {
		step_down();
	}
	return *this;
}

template class NodeIter<NodePtr>;
template class NodeIter<NodeView>;
template class NodeIter<FreeNodeView>;





template <typename NodePtrT>
ChildIter<NodePtrT>::ChildIter(const NodePtrT& nnode): node(nnode.child(0)) {

}

template <typename NodePtrT>
ChildIter<NodePtrT> ChildIter<NodePtrT>::operator++() {
	//cout << node.node << ' ' << node.node->parent->children + (BDIMS3-1) << endl;
	if (node.node == node.node->parent->children + (BDIMS3-1)) {
		node = node.parent().freechild();
	} else if (node.isfreenode()) {
		if (node.hasfreesibling()) {
			node = node.freesibling();
		} else {
			to_end();
		}
	} else {
		node = node.sibling(node.parentindex() + 1);
	}
	return *this;
}

template <>
ChildIter<NodePtr> ChildIter<NodePtr>::operator++() {
	if (node.node == node.node->parent->children + BDIMS3) {
		node = node.parent().freechild();
	} else if (node.isfreenode()) {
		if (node.hasfreesibling()) {
			node = node.freesibling();
		} else {
			to_end();
		}
	} else {
		node.node ++;
	}
	return *this;
}

template class ChildIter<NodePtr>;
template class ChildIter<NodeView>;
template class ChildIter<FreeNodeView>;


template <typename NodePtrT>
NodeIndex DirNodeIter<NodePtrT>::increment_func(NodeIndex nodepos) {
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

template class DirNodeIter<NodePtr>;
template class DirNodeIter<NodeView>;
template class DirNodeIter<FreeNodeView>;

/*
template class IHitCubeIter<NodePtr>;
template class IHitCubeIter<NodeView>;
template class IHitCubeIter<FreeNodeView>;

template class HitCubeIter<NodePtr>;
template class HitCubeIter<NodeView>;
template class HitCubeIter<FreeNodeView>;

template class HitBoxIter<NodePtr>;
template class HitBoxIter<NodeView>;
template class HitBoxIter<FreeNodeView>;

template class IHitCubeBlockIter<NodePtr>;
template class IHitCubeBlockIter<NodeView>;
template class IHitCubeBlockIter<FreeNodeView>;

template class HitCubeBlockIter<NodePtr>;
template class HitCubeBlockIter<NodeView>;
template class HitCubeBlockIter<FreeNodeView>;

template class HitBoxBlockIter<NodePtr>;
template class HitBoxBlockIter<NodeView>;
template class HitBoxBlockIter<FreeNodeView>;
*/
