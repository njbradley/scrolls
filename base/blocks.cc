#include "blocks.h"
#include "blockiter.h"

Block::Block() {
	
}

Block::Block(int nvalue): value(nvalue) {
	
}


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
		globalpos -= ivec3(parentindex()) * scale;
		scale *= BDIMS;
		node = node->parent;
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

bool NodeView::moveto(ivec3 pos, int goalscale) {
	ivec3 diff3 = pos - globalpos;
  int diffmax = std::max(std::max(diff3.x, diff3.y), diff3.z);
  int diffmin = std::min(std::min(diff3.x, diff3.y), diff3.z);
  while (diffmax >= scale or diffmin < 0 or scale < goalscale) {
    if (!step_up()) {
			return !(diffmax >= scale or diffmin < 0);
		}
    
    diff3 = pos - globalpos;
    diffmax = std::max(std::max(diff3.x, diff3.y), diff3.z);
    diffmin = std::min(std::min(diff3.x, diff3.y), diff3.z);
  }
  
  while (scale > goalscale) {
    ivec3 rem = (pos - globalpos) / (scale / BDIMS);
    if (!step_down(rem)) {
			return true;
		}
  }
	return true;
}

void NodeView::join() {
	delete[] node->children;
	node->flags &= ~Block::CONTINUES_FLAG;
	node->block = nullptr;
	on_change();
}

void NodeView::split() {
	if (node->block != nullptr) delete node->block;
	node->children = new Node[BDIMS3];
	for (int i = 0; i < BDIMS3; i ++) {
		node->children[i].parent = node;
	}
	node->flags |= Block::CONTINUES_FLAG;
	on_change();
}

bool NodeView::has_flag(uint32 flag) {
	return node->flags & flag;
}

void NodeView::set_flag(uint32 flag) {
	node->flags |= flag;
	flag &= Block::PROPOGATING_FLAGS;
	if (flag == 0) return;
	
	Node* curnode = node->parent;
	while (curnode != nullptr) {
		curnode->flags |= flag;
		curnode = curnode->parent;
	}
}

void NodeView::reset_flag(uint32 flag) {
	node->flags &= ~flag;
}

void NodeView::set_block(Block* block) {
	ASSERT(!continues());
	if (node->block != nullptr) delete node->block;
	node->block = block;
	on_change();
}

Block* NodeView::swap_block(Block* block) {
	ASSERT(!continues());
	Block* old = node->block;
	node->block = block;
	return old;
	on_change();
}

void NodeView::from_file(istream& ifile) {
	for (NodeView curnode : BlockIterable<BlockIter>(*this)) {
		if (ifile.peek() == '{') {
			ifile.get();
			curnode.split();
		} else if (ifile.peek() == '~') {
			ifile.get();
			curnode.set_block(nullptr);
		} else {
			curnode.set_block(new Block(ifile.get()));
		}
	}
}

void NodeView::to_file(ostream& ofile) {
	for (NodeView curnode : BlockIterable<BlockIter>(*this)) {
		if (curnode.continues()) {
			ofile.put('{');
		} else if (isnull()) {
			ofile.put('~');
		} else {
			ofile.put(curnode.node->block->value);
		}
	}
}

void NodeView::on_change() {
	set_flag(Block::RENDER_FLAG);
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
	// cout << "step down " << globalpos << ' ' << scale << endl;
	if (continues()) {
		NodeView::step_down(startpos());
		get_safe();
	} else {
		step_side();
	}
}

void NodeIter::step_side() {
	// cout << "step side " << globalpos << ' ' << scale << endl;
	if (node->parent == nullptr or scale >= max_scale) {
		finish();
	} else if (parentindex() == endpos()) {
		step_up();
		step_side();
	} else {
		NodeView::step_side(increment_func(parentindex()));
		get_safe();
	}
}

void NodeIter::get_safe() {
	// cout << "get safe " << globalpos << ' ' << scale << endl;
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
