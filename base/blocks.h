#ifndef SCROLLS_BLOCKS
#define SCROLLS_BLOCKS

#include "common.h"

// Number of times a block splits
#define BDIMS 2
// BDIMS^3
#define BDIMS3 8

struct Block {
	int value = 0;
	int renderindex = -1;
	uint8 lightval = 0;
	
	enum : uint32 {
		PROPOGATING_FLAGS = 0x0000ffff,
		RENDER_FLAG = 0x00000001,
		CONTINUES_FLAG = 0x00010000
	};
};

struct Node {
	Node* parent = nullptr;
	union {
		Node* children;
		Block* block;
	};
	uint32 flags = 0;
	
	Node();
};

// struct that represents an index into
// a inner node
struct NodeIndex {
	int index;
	
	NodeIndex(int index);
	NodeIndex(ivec3 pos);
	
	operator ivec3() const;
	operator int() const;
	
	int x() const;
	int y() const;
	int z() const;
};
	

// class that allows reading/modifying
// of the octree
//
class NodeView {
public:
	ivec3 globalpos;
	int scale;
	
	NodeView();
	NodeView(Node* node, ivec3 gpos, int nscale);
	
	bool isvalid() const;
	bool isnull() const;
	bool continues() const;
	
	NodeIndex parentindex() const;
	
	Block* block();
	const Block* block() const;
	
	void invalidate();
	
	bool step_down(NodeIndex pos);
	bool step_up();
	bool step_side(NodeIndex pos);
	
	NodeView get_global(ivec3 pos, int scale);
	bool moveto(ivec3 pos, int scale);
	
	void join();
	void split();
	
	bool has_flag(uint32 flag);
	void set_flag(uint32 flag);
	void reset_flag(uint32 flag);
	
	void on_change();
	
	void set_block(Block* block);
	Block* swap_block(Block* block);
	
	bool operator==(const NodeView& other) const;
	bool operator!=(const NodeView& other) const;
protected:
	Node* node = nullptr;
};

class BlockView : public NodeView {
public:
	BlockView();
	BlockView(const NodeView& node);
	
	Block* operator->();
	NodeView nodeview() const;
protected:
	using NodeView::step_down;
	using NodeView::step_up;
	using NodeView::step_side;
	using NodeView::moveto;
	
	using NodeView::join;
	using NodeView::split;
};

class NodeIter : public NodeView {
public:
	NodeIter(const NodeView& view);
	
	NodeIter operator++();
	NodeView& operator*();
	
	int max_scale = -1;
		
	// the start and end point for iterating inside a single block
  virtual NodeIndex startpos();
  virtual NodeIndex endpos();
	virtual NodeIndex increment_func(NodeIndex pos);
	
	void step_down();
	void step_side();
	void get_safe();
	
	// is the current node and all of its children valid
	virtual bool valid_tree() const;
	// is only the the current node valid
	virtual bool valid_node() const;
	
	virtual void finish();
};
	

class BlockContainer {
public:
	ivec3 globalpos;
	int scale;
	
	BlockContainer(ivec3 pos, int scale);
	
	BlockView get(ivec3 pos);
	NodeView get_global(ivec3 pos, int scale);
	
	NodeView rootview();
private:
	Node* root;
};


















// INLINE FUNCTIONS

inline Node::Node() {
	block = nullptr;
}

inline NodeIndex::NodeIndex(int ind): index(ind) {
	ASSERT(ind >= 0 and ind < BDIMS3);
}

inline NodeIndex::NodeIndex(ivec3 pos): index(pos.x*BDIMS*BDIMS + pos.y*BDIMS + pos.z) {
	ASSERT(pos.x < BDIMS and pos.x >= 0 and pos.y < BDIMS and pos.y >= 0 and pos.z < BDIMS and pos.z >= 0);
}

inline NodeIndex::operator int() const {
	return index;
}

inline NodeIndex::operator ivec3() const {
	return ivec3(index/(BDIMS*BDIMS), index/BDIMS%BDIMS, index%BDIMS);
}

inline int NodeIndex::x() const {
	return index/(BDIMS*BDIMS);
}

inline int NodeIndex::y() const {
	return index/BDIMS%BDIMS;
}

inline int NodeIndex::z() const {
	return index%BDIMS;
}





inline bool NodeView::isvalid() const {
	return scale >= 0;
}

inline bool NodeView::isnull() const {
	return node == nullptr;
}

inline bool NodeView::continues() const {
	return node->flags & Block::CONTINUES_FLAG;
}

inline NodeIndex NodeView::parentindex() const {
	return node - node->parent->children;
}

inline Block* NodeView::block() {
	return node->block;
}

inline const Block* NodeView::block() const {
	return node->block;
}

inline void NodeView::invalidate() {
	scale = -1;
	node = nullptr;
}

inline bool NodeView::operator==(const NodeView& other) const {
	return node == other.node;
}

inline bool NodeView::operator!=(const NodeView& other) const {
	return !(*this == other);
}



inline Block* BlockView::operator->() {
	return block();
}

inline NodeView BlockView::nodeview() const {
	return *this;
}


inline NodeIndex NodeIter::startpos() {
	return NodeIndex(0);
}

inline NodeIndex NodeIter::endpos() {
	return NodeIndex(BDIMS3-1);
}

inline NodeIndex NodeIter::increment_func(NodeIndex pos) {
	return int(pos) + 1;
}

// is the current node and all of its children valid
inline bool NodeIter::valid_tree() const {
	return true;
}

// is only the the current node valid
inline bool NodeIter::valid_node() const {
	return true;
}

inline void NodeIter::finish() {
	invalidate();
}

inline NodeView& NodeIter::operator*() {
	return *this;
}



#endif
