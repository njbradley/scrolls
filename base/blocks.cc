#include "blocks.h"
#include "blockiter.h"


bool NodePtr::step_down(NodeIndex pos) {
	if (haschildren()) {
		node = node->children + pos;
		return true;
	}
	return false;
}

bool NodePtr::step_down_free() {
	if (hasfreechild()) {
		node = node->freechild;
		return true;
	}
	return false;
}

bool NodePtr::step_up() {
	if (hasparent()) {
		node = node->parent;
		return true;
	}
	if (hasfreecontainer()) {
		node = node->freecontainer->highparent;
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

bool NodePtr::step_side_free() {
	if (hasnextfree()) {
		node = node->freecontainer->next;
		return true;
	}
	return false;
}

NodePtr NodePtr::child(NodeIndex index) const {
	NodePtr result = *this;
	if (result.step_down(index)) {
		return result;
	}
	return NodePtr();
}

NodePtr NodePtr::parent() const {
	NodePtr result = *this;
	if (result.step_up()) {
		return result;
	}
	return NodePtr();
}

NodePtr NodePtr::freechild() const {
	NodePtr result = *this;
	if (result.step_down_free()) {
		return result;
	}
	return NodePtr();
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

void NodePtr::subdivide() {
	Block* oldblock = node->block;
	node->block = nullptr;
	split();
	if (oldblock != nullptr) {
		for (int i = 0; i < BDIMS3; i ++) {
			node->children[i].block = new Block(*oldblock);
		}
	}
	delete oldblock;
}

void NodePtr::set_child(NodeIndex index, NodePtr newchild) {
	ASSERT(haschildren());
	newchild.node->parent = node;
	newchild.set_flag(Block::PARENT_FLAG);
	del_tree(&node->children[index]);
	node->children[index] = *newchild.node;
}

NodePtr NodePtr::swap_child(NodeIndex index, NodePtr newchild) {
	ASSERT(haschildren());
	newchild.node->parent = node;
	newchild.set_flag(Block::PARENT_FLAG);
	NodePtr old = child(index);
	node->children[index] = *newchild.node;
	return old;
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

// void NodePtr::from_file(istream& ifile) {
// 	// for (NodePtr curnode : BlockIterable<NodeIter<NodePtr>>(*this)) {
// 	// 	if (ifile.peek() == '{') {
// 	// 		ifile.get();
// 	// 		curnode.split();
// 	// 	} else if (ifile.peek() == '~') {
// 	// 		ifile.get();
// 	// 		curnode.set_block(nullptr);
// 	// 	} else {
// 	// 		curnode.set_block(new Block(ifile.get()));
// 	// 	}
// 	// }
// }
//
// void to_file_recursive(NodePtr node, vector<std::stringstream>& level_buffers, int depth) {
// 	if (level_buffers.size() >= depth) {
// 		level_buffers.emplace_back();
// 	}
// 	if (node.haschildren() {
// 		level_buffers[depth].put('{');
// 		for (int i = 0; i < BDIMS3; i ++) {
// 			to_file_recursive(node.child(i), level_buffers, depth + 1);
// 		}
// 	} else if (node.hasblock()) {
// 		level_buffers[depth].put(node.block()->value);
// 	} else {
// 		level_buffers[depth].put('~');
// 	}
// }
//
//
// void NodePtr::to_file(ostream& ofile) {
// 	vector<std::stringstream> level_buffers;
// 	to_file_recursive(node, level_buffers, 0);
// 	for (std::strinstream& buffer : level_buffers) {
// 		buffer.seekg(0, std::ios::end);
// 		uint32 len = htonl(buffer.tellg());
// 		buffer.seekg(0, std::ios::beg);
// 		ofile.write(&len, sizeof(len));
// 		ofile << buffer.rdbuf();
// 	}
// 	// for (NodePtr curnode : BlockIterable<NodeIter<NodePtr>>(*this)) {
// 	// 	if (curnode.haschildren()) {
// 	// 		ofile.put('{');
// 	// 	} else if (curnode.hasblock()) {
// 	// 		ofile.put(curnode.block()->value);
// 	// 	} else {
// 	// 		ofile.put('~');
// 	// 	}
// 	// }
// }

void NodePtr::copy_tree(Node* src, Node* dest) {
	*dest = *src;
	if (src->flags & Block::CHILDREN_FLAG) {
		dest->children = new Node[BDIMS3];
		for (int i = 0; i < BDIMS3; i ++) {
			copy_tree(&src->children[i], &dest->children[i]);
			dest->children[i].parent = dest;
		}
	} else if (src->block != nullptr) {
		dest->block = new Block(*src->block);
	}
	dest->flags |= Block::RENDER_FLAG;
}

void NodePtr::copy_tree(NodePtr other) {
	uint32 saved_flags = test_flag(Block::PARENT_FLAG | Block::FREENODE_FLAG);
	Node* oldparent = node->parent;
	del_tree(node);
	copy_tree(other.node, node);
	node->parent = oldparent;
	set_flag(saved_flags);
	on_change();
}

void NodePtr::swap_tree(NodePtr other) {
	uint32 saved_flags = test_flag(Block::PARENT_FLAG | Block::FREENODE_FLAG);
	uint32 other_saved_flags = other.test_flag(Block::PARENT_FLAG | Block::FREENODE_FLAG);
	reset_flag(saved_flags);
	other.reset_flag(other_saved_flags);
	std::swap(*node, *other.node);
	std::swap(node->parent, other.node->parent);
	if (haschildren()) {
		for (int i = 0; i < BDIMS3; i ++) {
			node->children[i].parent = node;
		}
	}
	if (other.haschildren()) {
		for (int i = 0; i < BDIMS3; i ++) {
			other.node->children[i].parent = other.node;
		}
	}
	set_flag(saved_flags);
	other.set_flag(other_saved_flags);
	parentindex();
	other.parentindex();
	set_flag(node->flags);
	other.set_flag(other.node->flags);
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





NodeView get_from_ptr(NodePtr node) {
	if (node.hasparent()) {
		NodeIndex index = node.parentindex();
		NodeView view = get_from_ptr(node.parent());
		view.step_down(index);
		return view;
	} else {
		return node.container()->root();
	}
}

// NodeView::NodeView(NodePtr node): NodeView(get_from_ptr(node)) {
//
// }

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

NodeView NodeView::freechild() const {
	NodeView result = *this;
	if (result.step_down_free()) {
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

void relocate_recursive(NodeView src, NodeView dest, int depth) {
	if (depth > 0) {
		if (!dest.haschildren()) {
			dest.split();
		}
		for (int i = 0; i < BDIMS3; i ++) {
			relocate_recursive(src, dest.child(i), depth-1);
		}
	} else {
		NodeView newdest = src.get_global(dest.position, dest.scale);
		if (newdest.isvalid()) {
			while (newdest.scale > dest.scale) {
				newdest.subdivide();
				newdest = newdest.get_global(dest.position, dest.scale);
			}
			dest.swap_tree(newdest);
		}
	}
}
			

BlockContainer BlockContainer::relocate(ivec3 newpos) {
	ivec3 diff = newpos - position;
	int resolution = scale;
	int depth = 0;
	while (diff.x / resolution == 0 and diff.y / resolution == 0 and diff.z / resolution) {
		resolution /= BDIMS;
		depth ++;
	}
	cout << "Moving with resolution " << resolution << ' ' << depth << endl;
	
	BlockContainer newroot (newpos, scale);
	relocate_recursive(root(), newroot.root(), depth);
	return newroot;
}
	
	
