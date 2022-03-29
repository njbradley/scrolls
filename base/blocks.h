#ifndef SCROLLS_BLOCKS
#define SCROLLS_BLOCKS

#include "common.h"

// Number of times a block splits
#define BDIMS 2
// BDIMS^3
#define BDIMS3 (BDIMS*BDIMS*BDIMS)

/*
All the voxel data is stored in a voxel tree, where there is a root node
the size of the world, which is divided into 8 smaller cubes, which
then keep dividing as needed to represent the world. The internal nodes
of the tree are represented as Node objects, and the leaf nodes
are Block objects.

Example:
+------------+
|+----++----+|
||    ||[][]||
||    ||[][]||
|+----++----+|
||[][]||    ||
||[][]||    ||
|+----++----+|
+------------+

In the example, every box is a node, and all the boxes that are not
subdivided would be the leaf nodes

When manipulating nodes in the tree, you will rarely interact
with raw Node* pointers, instead use the NodeView and similar
classes, which wrap a Node* pointer with helpful functions and
bookeeping of size and position
*/

struct Block {
	int value = 0;
	int renderindex = -1;
	uint8 sunlight = 0;
	uint8 blocklight = 0;
	
	enum : uint32 {
		PROPOGATING_FLAGS = 0x0000ffff,
		RENDER_FLAG = 0x00000001,
		CONTINUES_FLAG = 0x00010000
	};
	
	Block();
	Block(int value);
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
// can be used to convert between a position
// in child space (ie 0,1,0) to an index
// in the children array (ie 4)
struct NodeIndex {
	const int index;
	
	constexpr NodeIndex(int index);
	constexpr NodeIndex(ivec3 pos);
	
	constexpr operator ivec3() const;
	constexpr operator int() const;
	
	constexpr int x() const;
	constexpr int y() const;
	constexpr int z() const;
};
	

// class that allows reading/modifying
// of the octree
// A nodeview points to a position on the block tree, while
// also keeping track of the current position and size of the
// node pointed to. the view can be moved around by methods
// like step_up/down/side or moveto
// The node can also be modified using split/join and set methods
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
	
	NodeView child(NodeIndex index);
	NodeView get_global(ivec3 pos, int scale);
	bool moveto(ivec3 pos, int scale);
	
	void join();
	void split();
	
	bool has_flag(uint32 flag) const;
	void set_flag(uint32 flag);
	void reset_flag(uint32 flag);
	
	void on_change();
	
	void set_block(Block* block);
	Block* swap_block(Block* block);
	
	void from_file(istream& ifile);
	void to_file(ostream& ofile);
	
	bool operator==(const NodeView& other) const;
	bool operator!=(const NodeView& other) const;
// protected:
	Node* node = nullptr;
};

// BlockView is a more specific NodeView that is restricted
// to only leaf nodes. If constructed with a non leaf NodeView
// step_down is called until a block is found.
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
	

// This class owns a full voxel tree, that is located at globalpos and
// is scale big.
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

inline constexpr NodeIndex::NodeIndex(int ind): index(ind) {
	ASSERT(ind >= 0 and ind < BDIMS3);
}

inline constexpr NodeIndex::NodeIndex(ivec3 pos): index(pos.x*BDIMS*BDIMS + pos.y*BDIMS + pos.z) {
	ASSERT(pos.x < BDIMS and pos.x >= 0 and pos.y < BDIMS and pos.y >= 0 and pos.z < BDIMS and pos.z >= 0);
}

inline constexpr NodeIndex::operator int() const {
	return index;
}

inline constexpr NodeIndex::operator ivec3() const {
	return ivec3(index/(BDIMS*BDIMS), index/BDIMS%BDIMS, index%BDIMS);
}

inline constexpr int NodeIndex::x() const {
	return index/(BDIMS*BDIMS);
}

inline constexpr int NodeIndex::y() const {
	return index/BDIMS%BDIMS;
}

inline constexpr int NodeIndex::z() const {
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
