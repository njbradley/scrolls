#include "blocks.h"


NodeView::NodeView(): scale(-1) {
	
}

NodeView::NodeView(Node* nnode, ivec3 gpos, int nscale): node(nnode), globalpos(gpos), scale(nscale) {
	
}

bool NodeView::step_down(NodeIndex pos) {
	if (node != nullptr and continues()) {
		node = node->children + pos;
		scale /= BDIMS;
		globalpos += scale * ivec3(pos);
		return true;
	}
	return false;
}

bool NodeView::step_up() {
	if (node->parent != nullptr) {
		node = node->parent;
		globalpos -= ivec3(parentindex()) * scale;
		scale *= BDIMS;
		return true;
	}
	return false;
}

bool NodeView::step_side(NodeIndex pos) {
	if (node->parent != nullptr) {
		globalpos += ivec3(pos) * scale - ivec3(parentindex()) * scale;
		node = node + (int(pos) - int(parentindex()));
		return true;
	}
	return false;
}

NodeView NodeView::get_global(ivec3 pos, int scale) {
	NodeView result = *this;
	if (result.moveto(pos, scale)) {
		return result;
	}
	return NodeView();
}

bool NodeView::moveto(ivec3 pos, int scale) {
	ivec3 diff3 = pos - globalpos;
  int diffmax = std::max(std::max(diff3.x, diff3.y), diff3.z);
  int diffmin = std::min(std::min(diff3.x, diff3.y), diff3.z);
  while (diffmax >= scale or diffmin < 0 or scale < scale) {
    if (!step_up()) {
			return false;
		}
    
    diff3 = pos - globalpos;
    diffmax = std::max(std::max(diff3.x, diff3.y), diff3.z);
    diffmin = std::min(std::min(diff3.x, diff3.y), diff3.z);
  }
  
  while (scale > scale) {
    ivec3 rem = (pos - globalpos) / (scale / BDIMS);
    if (!step_down(rem)) {
			return false;
		}
  }
	return true;
}

void NodeView::join() {
	delete[] node->children;
	node->flags &= ~Block::CONTINUES_FLAG;
	node->block = nullptr;
}

void NodeView::split() {
	if (node->block != nullptr) delete node->block;
	node->children = new Node[BDIMS3];
}


BlockView::BlockView() {
	
}

BlockView::BlockView(const NodeView& view): NodeView(view) {
	if (isvalid()) {
		while (continues()) {
			step_down(0);
		}
		ASSERT(block() != nullptr);
	}
}











NodeIter::NodeIter(const NodeView& view): NodeView(view), max_scale(view.scale) {
	
}


void NodeIter::step_down() {
	if (continues()) {
		NodeView::step_down(startpos());
		get_safe();
	} else {
		step_side();
	}
}

void NodeIter::step_side() {
	if (node->parent == nullptr or scale >= max_scale) {
		finish();
	} else if (parentindex() == endpos()) {
		step_side();
	} else {
		NodeView::step_side(increment_func(parentindex()));
		get_safe();
	}
}

void NodeIter::get_safe() {
	if (!valid_tree()) {
		step_side();
	} else if (!valid_node()) {
		step_down();
	}
}

NodeIter NodeIter::operator++() {
	if (isvalid()) {
		step_down();
	}
	return *this;
}



BlockContainer::BlockContainer(ivec3 gpos, int nscale): globalpos(gpos), scale(nscale) {
	root = new Node();
	root->block = nullptr;
}

BlockView BlockContainer::get(ivec3 pos) {
	NodeView result (root, globalpos, scale);
	return BlockView(result.get_global(pos, 1));
}

NodeView BlockContainer::get_global(ivec3 pos, int w) {
	NodeView result (root, globalpos, scale);
	return result.get_global(pos, w);
}

NodeView BlockContainer::rootview() {
	return NodeView(root, globalpos, scale);
}
