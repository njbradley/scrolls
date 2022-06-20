#include "blocks.h"
#include "blockiter.h"



NodePtr NodePtr::child(NodeIndex index) const {
	if (haschildren()) {
		return NodePtr(node->children + index);
	}
	return NodePtr();
}

NodePtr NodePtr::sibling(NodeIndex index) const {
	if (hasparent()) {
		return NodePtr(node + (int(index) - int(parentindex())));
	}
	return NodePtr();
}

NodePtr NodePtr::parent() const {
	if (hasparent()) {
		return NodePtr(node->parent);
	}
	return NodeView();
}

NodePtr NodePtr::freeparent() const {
	if (isfreenode()) {
		return NodePtr(freenode()->highparent);
	}
	return NodeView();
}

NodePtr NodePtr::freechild() const {
	if (hasfreechild()) {
		return NodePtr(node->freechild);
	}
	return NodePtr();
}

NodePtr NodePtr::freesibling() const {
	if (hasfreesibling()) {
		return NodePtr(freenode()->next);
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

void NodePtr::remove_freechild() {
	if (isfreenode()) {
		freenode()->highparent->freechild = freenode()->next;
		del_tree(node);
		delete freenode();
		invalidate();
	}
}

void NodePtr::add_freechild() {
	FreeNode* newfree = new FreeNode();
	newfree->next = node->freechild;
	newfree->highparent = node;
	node->freechild = newfree;
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






NodeView NodeView::child(NodeIndex index) const {
	if (haschildren()) {
		return NodeView(
			node->children + index,
			position + scale / BDIMS * ivec3(index),
			scale / BDIMS,
			(RefCounted<NodeView>*)(highparent)
		);
	}
	return NodeView();
}

NodeView NodeView::sibling(NodeIndex index) const {
	if (hasparent()) {
		return NodeView(
			node + (int(index) - int(parentindex())),
			position + ivec3(index) * scale - ivec3(parentindex()) * scale,
			scale,
			highparent
		);
	}
	return NodeView();
}

NodeView NodeView::parent() const {
	if (hasparent()) {
		return NodeView(
			node->parent,
			position - ivec3(parentindex()) * scale,
			scale * BDIMS,
			highparent
		);
	}
	return NodeView();
}

NodeView NodeView::freeparent() const {
	if (isfreenode()) {
		return *highparent;
	}
	return NodeView();
}

NodeView NodeView::freechild() const {
	if (hasfreechild()) {
		return NodeView(
			node->freechild,
			ivec3(0,0,0),
			scale / BDIMS,
			new RefCounted<NodeView>(*this)
		);
	}
	return NodeView();
}

NodeView NodeView::freesibling() const {
	if (hasfreesibling()) {
		return NodeView(
			freenode()->next,
			ivec3(0,0,0),
			scale,
			highparent
		);
	}
	return NodeView();
}

NodeView NodeView::get_global(ivec3 pos, int goalscale) {
	NodeView result = *this;
	IHitCube goalbox (pos, goalscale);
	while (!result.contains(goalbox)) {
		if (!result.hasparent()) {
			if (result.isfreenode()) {
				return NodeView();
			} else if (result.hascontainer()) {
				return result.container()->get_global(pos, goalscale);
			}
		}
		result = result.parent();
	}
	
	while (result.scale > goalscale and result.haschildren()) {
    NodeIndex rem = (pos - result.position) / (result.scale / BDIMS);
		result = result.child(rem);
  }
	return result;
}



FreeNodeView::FreeNodeView(NodePtr node, ivec3 lpos, int nscale, RefCounted<FreeNodeView>* hparent):
NodePtr(node), HitCube(hparent->transform_out(HitCube(lpos, nscale, quat(1,0,0,0)))), localpos(lpos), highparent(hparent) {
	highparent->incref();
}



FreeNodeView FreeNodeView::child(NodeIndex index) const {
	if (haschildren()) {
		return FreeNodeView(
			node->children + index,
			localpos + scale / BDIMS * ivec3(index),
			scale / BDIMS,
			highparent
		);
	}
	return FreeNodeView();
}

FreeNodeView FreeNodeView::sibling(NodeIndex index) const {
	if (hasparent()) {
		return FreeNodeView(
			node + (int(index) - int(parentindex())),
			localpos + ivec3(index) * scale - ivec3(parentindex()) * scale,
			scale,
			highparent
		);
	}
	return FreeNodeView();
}

FreeNodeView FreeNodeView::parent() const {
	if (hasparent()) {
		return FreeNodeView(
			node->parent,
			localpos - ivec3(parentindex()) * scale,
			scale * BDIMS,
			highparent
		);
	}
	return FreeNodeView();
}

FreeNodeView FreeNodeView::freeparent() const {
	if (isfreenode()) {
		return *highparent;
	}
	return FreeNodeView();
}

FreeNodeView FreeNodeView::freechild() const {
	if (hasfreechild()) {
		return FreeNodeView(
			node->freechild,
			ivec3(0,0,0),
			scale / BDIMS,
			new RefCounted<FreeNodeView>(*this)
		);
	}
	return FreeNodeView();
}

FreeNodeView FreeNodeView::freesibling() const {
	if (hasfreesibling()) {
		return FreeNodeView(
			freenode()->next,
			ivec3(0,0,0),
			scale,
			highparent
		);
	}
	return FreeNodeView();
}




BlockView::BlockView() {
	
}

BlockView::BlockView(const NodeView& view): NodeView(view) {
	ASSERT(hasblock());
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
	copy_tree(other);
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
	IHitCube goalbox (pos, goal_scale);
	if (contains(goalbox)) {
		return NodeView::get_global(pos, goal_scale);
	}
	
	BlockContainer* other = find_neighbor(pos, goal_scale);
	if (other != nullptr) {
		return other->get_global(pos, goal_scale);
	}
	return NodeView();
}
	
