#include "blocks.h"
#include "blockiter.h"


bool NodePtr::step_down(NodeIndex pos) {
	if (haschildren()) {
		node = node->children + pos;
		return true;
	}
	return false;
}

bool NodePtr::step_up() {
	if (hasparent()) {
		node = node->parent;
		return true;
	}
	return false;
}

bool NodePtr::step_side(NodeIndex pos) {
	if (hasparent()) {
		node = node + (int(pos) - int(parentindex()));
		return true;
	}
	return false;
}

NodePtr NodePtr::child(NodeIndex index) const {
	NodePtr result = *this;
	if (result.step_down(index)) {
		return result;
	}
	return NodeView();
}

NodePtr NodePtr::parent() const {
	NodePtr result = *this;
	if (result.step_up()) {
		return result;
	}
	return NodeView();
}


void NodePtr::join() {
	del_tree(node);
	node->flags &= ~Block::CHILDREN_FLAG;
	node->block = nullptr;
	on_change();
}

void NodePtr::split() {
	if (node->block != nullptr) delete node->block;
	node->children = new Node[BDIMS3];
	for (int i = 0; i < BDIMS3; i ++) {
		node->children[i].parent = node;
		node->children[i].flags |= Block::PARENT_FLAG;
	}
	node->flags |= Block::CHILDREN_FLAG;
	on_change();
}

void NodePtr::set_flag(uint32 flag) {
	node->flags |= flag;
	flag &= Block::PROPOGATING_FLAGS;
	if (flag == 0) return;
	
	Node* curnode = node;
	while (curnode->flags & Block::PARENT_FLAG) {
		curnode = curnode->parent;
		curnode->flags |= flag;
	}
}

void NodePtr::reset_flag(uint32 flag) {
	node->flags &= ~flag;
}

void NodePtr::set_block(Block* block) {
	ASSERT(!haschildren());
	if (node->block != nullptr) delete node->block;
	node->block = block;
	on_change();
}

Block* NodePtr::swap_block(Block* block) {
	ASSERT(!haschildren());
	Block* old = node->block;
	node->block = block;
	return old;
	on_change();
}

void NodePtr::from_file(istream& ifile) {
	// for (NodeView curnode : BlockIterable<NodeIter>(*this)) {
	// 	if (ifile.peek() == '{') {
	// 		ifile.get();
	// 		curnode.split();
	// 	} else if (ifile.peek() == '~') {
	// 		ifile.get();
	// 		curnode.set_block(nullptr);
	// 	} else {
	// 		curnode.set_block(new Block(ifile.get()));
	// 	}
	// }
}

void NodePtr::to_file(ostream& ofile) {
	// for (NodeView curnode : BlockIterable<NodeIter>(*this)) {
	// 	if (curnode.haschildren()) {
	// 		ofile.put('{');
	// 	} else if (curnode.hasblock()) {
	// 		ofile.put(curnode.node->block->value);
	// 	} else {
	// 		ofile.put('~');
	// 	}
	// }
}

void NodePtr::copy_tree(Node* src, Node* dest) {
	*dest = *src;
	if (src->flags & Block::CHILDREN_FLAG) {
		dest->children = new Node[BDIMS3];
		for (int i = 0; i < BDIMS3; i ++) {
			dest->children[i].parent = dest;
			copy_tree(&src->children[i], &dest->children[i]);
		}
	} else if (src->block != nullptr) {
		dest->block = new Block(*node->block);
	}
}

void NodePtr::copy_tree(NodePtr other) {
	del_tree(node);
	copy_tree(other.node, node);
	on_change();
}

void NodePtr::swap_tree(NodePtr other) {
	bool parent_flag = test_flag(Block::PARENT_FLAG);
	bool other_parent_flag = test_flag(Block::PARENT_FLAG);
	reset_flag(Block::PARENT_FLAG);
	other.reset_flag(Block::PARENT_FLAG);
	std::swap(*node, *other.node);
	std::swap(node->parent, other.node->parent);
	if (parent_flag) set_flag(Block::PARENT_FLAG);
	if (other_parent_flag) other.set_flag(Block::PARENT_FLAG);
	on_change();
}

void NodePtr::on_change() {
	set_flag(Block::RENDER_FLAG);
}

void NodePtr::del_tree(Node* node) {
	if (node->flags & Block::CHILDREN_FLAG) {
		for (int i = 0; i < BDIMS3; i ++) {
			del_tree(&node->children[i]);
		}
		delete[] node->children;
	} else if (node->block != nullptr) {
		delete node->block;
	}
	node->children = nullptr;
}









bool NodeView::step_down(NodeIndex pos) {
	if (haschildren()) {
		node = node->children + pos;
		scale /= BDIMS;
		position += scale * ivec3(pos);
		return true;
	}
	return false;
}

bool NodeView::step_up() {
	if (hasparent()) {
		position -= ivec3(parentindex()) * scale;
		scale *= BDIMS;
		node = node->parent;
		return true;
	}
	return false;
}

bool NodeView::step_side(NodeIndex pos) {
	if (hasparent()) {
		position += ivec3(pos) * scale - ivec3(parentindex()) * scale;
		node = node + (int(pos) - int(parentindex()));
		return true;
	}
	return false;
}

NodeView NodeView::child(NodeIndex index) const {
	NodeView result = *this;
	if (result.step_down(index)) {
		return result;
	}
	return NodeView();
}

NodeView NodeView::parent() const {
	NodeView result = *this;
	if (result.step_up()) {
		return result;
	}
	return NodeView();
}

NodeView NodeView::get_global(ivec3 pos, int goalscale) {
	NodeView result = *this;
	if (result.moveto(pos, goalscale)) {
		return result;
	}
	return NodeView();
}

bool NodeView::moveto(ivec3 pos, int goalscale) {
	IHitCube goalbox (pos, goalscale);
	while (!contains(goalbox)) {
    if (!step_up()) {
			if (!goalbox.contains(*this) and node->container != nullptr) {
				NodeView othernode = node->container->get_global(pos, goalscale);
				if (othernode.isvalid()) {
					*this = othernode;
					return true;
				}
				return false;
			}
			return true;
		}
  }
  
  while (scale > goalscale) {
    ivec3 rem = (pos - position) / (scale / BDIMS);
    if (!step_down(rem)) {
			return true;
		}
  }
	return true;
}










BlockView::BlockView() {
	
}

BlockView::BlockView(const NodeView& view): NodeView(view) {
	if (isvalid()) {
		while (haschildren()) {
			step_down(0);
		}
		ASSERT(block() != nullptr);
	}
}











NodeIter::NodeIter(const NodeView& view): NodeView(view), max_scale(view.scale) {
	
}


void NodeIter::step_down() {
	// cout << "step down " << position << ' ' << scale << endl;
	if (haschildren()) {
		NodeView::step_down(startpos());
		get_safe();
	} else {
		step_side();
	}
}

void NodeIter::step_side() {
	// cout << "step side " << position << ' ' << scale << endl;
	if (!hasparent() or scale >= max_scale) {
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
	// cout << "get safe " << position << ' ' << scale << endl;
	if (!valid_tree()) {
		// cout << "invalid tree" << endl;
		step_side();
	} else if (!valid_node()) {
		// cout << "invalid node" << endl;
		step_down();
	}
}

NodeIter NodeIter::operator++() {
	if (isvalid()) {
		step_down();
	}
	return *this;
}



BlockContainer::BlockContainer(ivec3 gpos, int nscale): NodeView(new Node(), gpos, nscale) {
	reset_flag(Block::PARENT_FLAG);
	node->container = this;
}

BlockContainer::~BlockContainer() {
	if (node != nullptr) {
		del_tree(node);
		delete node;
	}
}

BlockContainer::BlockContainer(const BlockContainer& other): NodeView(other) {
	node = new Node();
	root().copy_tree(other.root());
	node->container = this;
}

BlockContainer::BlockContainer(BlockContainer&& other) {
	swap(other);
}

BlockContainer& BlockContainer::operator=(BlockContainer other) {
	swap(other);
	return *this;
}

void BlockContainer::swap(BlockContainer& other) {
	std::swap(node, other.node);
	std::swap(position, other.position);
	std::swap(scale, other.scale);
	if (node != nullptr) node->container = this;
	if (other.node != nullptr) other.node->container = &other;
}

BlockContainer* BlockContainer::find_neighbor(ivec3 pos, int goalscale) {
	return nullptr;
}

NodeView BlockContainer::get_global(ivec3 pos, int goal_scale) {
	NodeView node = root();
	IHitCube goalbox (pos, goal_scale);
	if (node.contains(goalbox)) {
		if (node.moveto(pos, goal_scale)) {
			return node;
		}
		return NodeView();
	}
	
	BlockContainer* other = find_neighbor(pos, goal_scale);
	if (other != nullptr) {
		return other->NodeView::get_global(pos, goal_scale);
	}
	return NodeView();
}

NodeView BlockContainer::root() const {
	return NodeView(*this);
}
