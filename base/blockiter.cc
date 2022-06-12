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
	} else {
		step_side();
	}
}

template <typename NodePtrT>
void NodeIter<NodePtrT>::step_side() {
	// cout << "step side " << position << ' ' << scale << endl;
	if (!node.hasparent() or node == highest_node) {
		finish();
	} else if (node.parentindex() == endpos()) {
		// node.step_up();
		node = node.parent();
		step_side();
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
