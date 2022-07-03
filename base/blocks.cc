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
	update_depth();
	on_change();
}

void NodePtr::split() {
	if (node->block != nullptr) delete node->block;
	node->children = new Node[BDIMS3];
	for (int i = 0; i < BDIMS3; i ++) {
		update_child(&node->children[i]);
	}
	node->flags |= Block::CHILDREN_FLAG;
	update_depth();
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
		node->parent->freechild = freenode()->next;
		parent().update_depth();
		del_tree(node);
		delete freenode();
		invalidate();
	}
}

void NodePtr::add_freechild(vec3 offset, quat rotation) {
	FreeNode* newfree = new FreeNode();
	newfree->offset = offset;
	newfree->rotation = rotation;
	newfree->next = node->freechild;
	node->freechild = newfree;
	update_child(newfree);
	update_depth();
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
			NodePtr(dest).update_child(&dest->children[i]);
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
	if (hasparent()) {
		parent().update_depth();
	}
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
			update_child(&node->children[i]);
		}
	}
	if (other.haschildren()) {
		for (int i = 0; i < BDIMS3; i ++) {
			other.update_child(&other.node->children[i]);
		}
	}
	set_flag(saved_flags);
	other.set_flag(other_saved_flags);
	parentindex();
	other.parentindex();
	set_flag(node->flags);
	other.set_flag(other.node->flags);
	if (hasparent()) {
		parent().update_depth();
	}
}

void NodePtr::on_change() {
	set_flag(Block::RENDER_FLAG);
}

void NodePtr::update_child(Node* child) {
	child->parent = node;
	child->flags |= Block::PARENT_FLAG;
}

void NodePtr::update_depth() {
	uint8 new_depth = 0;
	if (haschildren()) {
		for (int i = 0; i < BDIMS3; i ++) {
			new_depth = std::max(new_depth, uint8(node->children[i].max_depth+1));
		}
	}
	if (hasfreechild()) {
		for (FreeNode* free = node->freechild; free != nullptr; free = free->next) {
			new_depth = std::max(new_depth, uint8(free->max_depth+1));
		}
	}
	update_depth(new_depth);
}

void NodePtr::update_depth(uint8 new_depth) {
	if (new_depth != node->max_depth) {
		node->max_depth = new_depth;
		if (hasparent()) {
			parent().update_depth();
		}
	}
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


BlockIterable<ChildIter<NodePtr>> NodePtr::children() {
	return iter<ChildIter>();
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
	if (hassiblings()) {
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
		if (isfreenode()) {
			return *highparent;
		}
		return NodeView(
			node->parent,
			position - ivec3(parentindex()) * scale,
			scale * BDIMS,
			highparent
		);
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
		if (!result.hasparent() and result.hascontainer()) {
			return result.container()->get_global(pos, goalscale);
		} else if (result.isfreenode()) {
			return NodeView();
		}
		result = result.parent();
	}
	
	while (result.scale > goalscale and result.haschildren()) {
    NodeIndex rem = (pos - result.position) / (result.scale / BDIMS);
		result = result.child(rem);
  }
	return result;
}

BlockIterable<ChildIter<NodeView>> NodeView::children() {
	return iter<ChildIter>();
}



HitCube transform_out_hparent(HitCube input, RefCounted<FreeNodeView>* hparent) {
	if (hparent != nullptr) {
		return hparent->transform_out(input);
	}
	return input;
}

FreeNodeView::FreeNodeView(NodePtr node, ivec3 lpos, int nscale, RefCounted<FreeNodeView>* hparent):
NodePtr(node), HitCube(transform_out_hparent(HitCube(lpos, nscale, quat(1,0,0,0)), hparent)), localpos(lpos), highparent(hparent) {
	
}

FreeNodeView::FreeNodeView(const NodeView& nodeview): NodePtr(nodeview), HitCube(nodeview.position, nodeview.scale, quat(1,0,0,0)), localpos(nodeview.position), highparent(nullptr) {

}

void FreeNodeView::recalculate_position() {
	if (highparent != nullptr) {
		HitCube::operator=(highparent->transform_out(HitCube(localpos, scale, quat(1,0,0,0))));
	} else {
		HitCube::operator=(HitCube(localpos, scale, quat(1,0,0,0)));
	}
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
	if (hassiblings()) {
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
		if (isfreenode()) {
			return FreeNodeView(highparent->node, highparent->localpos, highparent->scale, highparent->highparent);
		}
		return FreeNodeView(
			node->parent,
			localpos - ivec3(parentindex()) * scale,
			scale * BDIMS,
			highparent
		);
	}
	return FreeNodeView();
}

FreeNodeView FreeNodeView::freechild() const {
	if (hasfreechild()) {
		RefCounted<FreeNodeView>* hparent = new RefCounted<FreeNodeView>(*this);
		hparent->position += node->freechild->offset;
		hparent->rotation *= node->freechild->rotation;
		return FreeNodeView(
			node->freechild,
			ivec3(0,0,0),
			scale / BDIMS,
			hparent
		);
	}
	return FreeNodeView();
}

FreeNodeView FreeNodeView::freesibling() const {
	if (hasfreesibling()) {
		RefCounted<FreeNodeView>* hparent = new RefCounted<FreeNodeView>(highparent->node, highparent->localpos, highparent->scale, highparent->highparent);
		hparent->position += freenode()->next->offset;
		hparent->rotation *= freenode()->next->rotation;
		return FreeNodeView(
			freenode()->next,
			ivec3(0,0,0),
			scale,
			hparent
		);
	}
	return FreeNodeView();
}

FreeNodeView::operator NodeView() const {
	return NodeView(node, localpos, scale);
}

BlockIterable<ChildIter<FreeNodeView>> FreeNodeView::children() {
	return iter<ChildIter>();
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
	
