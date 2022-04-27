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


For example, the block structure here:

+------------+
|+----++----+|
||    ||[][]||
||    ||[][]||
|+----++----+|
||[][]||    ||
||[][]||    ||
|+----++----+|
+------------+

Gives the tree:

          +------------+
          |            |
          |            |
          |            |
          |            |
          |            |
          |            |
          |            |
          +------------+
       /     |      |     \
  +----+  +----+  +----+  +----+
  |    |  |    |  |    |  |    |
  |    |  |    |  |    |  |    |
  +----+  +----+  +----+  +----+
          / | | \         / | | \
        [] [] [] []     [] [] [] []

In the example, every box is a node, and all the boxes that are not
subdivided would be the leaf nodes. The example is also in 2d so there are
only 4 children, but real nodes are 3d and have 8 children

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
		CHILDREN_FLAG = 0x00010000,
		PARENT_FLAG = 0x00020000
	};
	
	Block();
	Block(int value);
};

struct Node {
	Node* parent = nullptr;
	union {
		Node* children = nullptr;
		Block* block;
	};
	uint32 flags = 0;
	int lastval = 1;
	
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
	
	// whether this view points to a real node
	// default constructed NodeViews will return false,
	// and some methods that can fail (get_global, child)
	// will return an instance that is not valid on failure
	bool isvalid() const;
	operator bool() const;
	// whether the node has a block (accessed by block())
	// hasblock() and haschildren() cannot both be true
	bool hasblock() const;
	// whether the node has children (accesed by child())
	bool haschildren() const;
	bool hasparent() const;
	
	// the index where this node is in the parent node
	// ie: node.parent().child(node.parentindex()) == node
	NodeIndex parentindex() const;
	
	// the block stored at this node
	// warning: if no block is stored, a possibly random pointer will
	// be returned, so be sure to access this only when sure there
	// is a block (when hasblock() is true)
	Block* block();
	const Block* block() const;
	
	// turns the current view into an invaid instance
	// basically the same as node = NodeView()
	void invalidate();
	
	// moves the view down, up, or sideways on the tree.
	// returns true if the movement was successful.
	bool step_down(NodeIndex pos);
	bool step_up();
	bool step_side(NodeIndex pos);
	
	// returns a new view to the parent node
	NodeView parent();
	// returns a new view to the child at the given index.
	NodeView child(NodeIndex index);
	// returns a view of the block at the given position
	// if the position is outside of the root node of the tree,
	// an invalid view is returned.
	// if the position is inside the root node but there isnt
	// a node with the given scale, the smallest node will be returned
	// this means passing 1 as scale guarantees you will recieve a leaf node
	NodeView get_global(ivec3 pos, int scale);
	// behaves similar to get_global, this method will try to move
	// the current view to the given position
	// if the position is outside the root node, false is returned.
	// the view will be left pointing at the root node
	bool moveto(ivec3 pos, int scale);
	
	// turns a leaf node into an inner node, and
	// creates 8 child nodes
	void join();
	// turns an inner node into a leaf node, deleting
	// all children previously on the node
	void split();
	
	// these methods read/write/modify the flags set on nodes,
	// where flag is a bit mask. the flags are defined in the
	// block class,
	bool test_flag(uint32 flag) const;
	void set_flag(uint32 flag);
	void reset_flag(uint32 flag);
	
	// this method should be called whenever the structure or value of
	// a node is changed
	void on_change();
	
	// modify the block that this node holds.
	void set_block(Block* block);
	Block* swap_block(Block* block);
	
	// read/write the current nodes tree to/from a file.
	void from_file(istream& ifile);
	void to_file(ostream& ofile);
	
	// swaps the tree at the current node with the tree pointed to
	// by other
	void swap_tree(NodeView other);
	// deletes the current tree and copies the tree pointed to
	// by other onto this node
	void copy_tree(NodeView other);
	
	bool operator==(const NodeView& other) const;
	bool operator!=(const NodeView& other) const;
// protected:
	Node* node = nullptr;
	
	void copy_tree(Node* src, Node* dest);
	void del_tree(Node* node);
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
	

// this is the class that allocates and owns an octree
// and all nodes
class BlockContainer : protected NodeView {
public:
	BlockContainer(ivec3 pos, int scale);
	BlockContainer(const BlockContainer& other);
	BlockContainer(BlockContainer&& other);
	~BlockContainer();
	
	BlockContainer& operator=(BlockContainer other);
	
	void swap(BlockContainer& other);
	
	using NodeView::globalpos;
	using NodeView::scale;
	
	using NodeView::hasblock;
	using NodeView::haschildren;
	using NodeView::block;
	using NodeView::child;
	using NodeView::get_global;
	using NodeView::split;
	using NodeView::join;
	using NodeView::test_flag;
	using NodeView::set_flag;
	using NodeView::reset_flag;
	using NodeView::set_block;
	using NodeView::swap_block;
	using NodeView::from_file;
	using NodeView::to_file;
	
	NodeView root() const;
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

inline bool NodeView::hasblock() const {
	return !haschildren() and node->block != nullptr;
}

inline bool NodeView::haschildren() const {
	return node->flags & Block::CHILDREN_FLAG;
}

inline bool NodeView::hasparent() const {
	// return node->parent != nullptr;
	return node->flags & Block::PARENT_FLAG;
}

inline bool NodeView::test_flag(uint32 flag) const {
	return node->flags & flag;
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
