#ifndef SCROLLS_BLOCKS
#define SCROLLS_BLOCKS

#include "common.h"

#include "entity.h"

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
	union {
		Node* parent = nullptr;
		BlockContainer* container;
		FreeNode* freecontainer;
	};
	union {
		Node* children = nullptr;
		Block* block;
	};
	FreeNode* freenode = nullptr;
	uint32 flags = 0;
	int lastval = 1;
};

struct FreeNode : Node {
	FreeNode* next = nullptr;
	quat rotation;
	vec3 offset;
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
	

class NodePtr {
public:
	
	NodePtr();
	NodePtr(Node* node);
	
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
	
	BlockContainer* container();
	FreeNode* freecontainer();
	
	// turns the current view into an invaid instance
	// basically the same as node = NodeView()
	void invalidate();
	
	// moves the view down, up, or sideways on the tree.
	// returns true if the movement was successful.
	bool step_down(NodeIndex pos);
	bool step_up();
	bool step_side(NodeIndex pos);
	
	// returns a new view to the parent node
	NodePtr parent() const;
	// returns a new view to the child at the given index.
	NodePtr child(NodeIndex index) const;
	
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
	void swap_tree(NodePtr other);
	// deletes the current tree and copies the tree pointed to
	// by other onto this node
	void copy_tree(NodePtr other);
	
	bool operator==(const NodePtr& other) const;
	bool operator!=(const NodePtr& other) const;
protected:
	Node* node = nullptr;
	
	void copy_tree(Node* src, Node* dest);
	void del_tree(Node* node);
};



// class that allows reading/modifying
// of the octree
// A nodeview points to a position on the block tree, while
// also keeping track of the current position and size of the
// node pointed to. the view can be moved around by methods
// like step_up/down/side or moveto
// The node can also be modified using split/join and set methods
class NodeView : public NodePtr, public IHitCube {
public:
	
	NodeView();
	NodeView(NodePtr node, IHitCube cube);
	NodeView(NodePtr node, ivec3 gpos, int nscale);
	// explicit NodeView(NodePtr node);
	
	// moves the view down, up, or sideways on the tree.
	// returns true if the movement was successful.
	bool step_down(NodeIndex pos);
	bool step_up();
	bool step_side(NodeIndex pos);
	
	// returns a new view to the parent node
	NodeView parent() const;
	// returns a new view to the child at the given index.
	NodeView child(NodeIndex index) const;
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
};


class FreeNodeView : public NodeView {
	FreeNode* freecontainer = nullptr;
	
	FreeNodeView();
	FreeNodeView(const NodeView& node, FreeNode* freecont);
	explicit FreeNodeView(const NodeView& node);
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
	
	using NodeView::position;
	using NodeView::scale;
	
	using NodeView::hasblock;
	using NodeView::haschildren;
	using NodeView::block;
	using NodeView::child;
	using NodeView::split;
	using NodeView::join;
	using NodeView::test_flag;
	using NodeView::set_flag;
	using NodeView::reset_flag;
	using NodeView::set_block;
	using NodeView::swap_block;
	using NodeView::from_file;
	using NodeView::to_file;
	
	virtual BlockContainer* find_neighbor(ivec3 pos, int goalscale);
	NodeView get_global(ivec3 pos, int goal_scale);
	
	NodeView root() const;
};



















// INLINE FUNCTIONS

inline Block::Block() {
	
}

inline Block::Block(int nvalue): value(nvalue) {
	
}



inline constexpr NodeIndex::NodeIndex(int ind): index(ind) {
	ASSERT(ind >= 0 and ind < BDIMS3);
}

inline constexpr NodeIndex::NodeIndex(ivec3 pos): index(pos.x*BDIMS*BDIMS + pos.y*BDIMS + pos.z) {
	// std::cout << pos << std::endl;
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



inline NodePtr::NodePtr(): node(nullptr) {
	
}

inline NodePtr::NodePtr(Node* nnode): node(nnode) {
	
}

inline bool NodePtr::isvalid() const {
	return node != nullptr;
}

inline bool NodePtr::hasblock() const {
	return !haschildren() and node->block != nullptr;
}

inline bool NodePtr::haschildren() const {
	return node->flags & Block::CHILDREN_FLAG;
}

inline bool NodePtr::hasparent() const {
	return node->flags & Block::PARENT_FLAG;
}

inline bool NodePtr::test_flag(uint32 flag) const {
	return node->flags & flag;
}

inline NodeIndex NodePtr::parentindex() const {
	return node - node->parent->children;
}

inline Block* NodePtr::block() {
	return node->block;
}

inline const Block* NodePtr::block() const {
	return node->block;
}

inline BlockContainer* NodePtr::container() {
	return node->container;
}

inline FreeNode* NodePtr::freecontainer() {
	return node->freecontainer;
}

inline void NodePtr::invalidate() {
	node = nullptr;
}

inline bool NodePtr::operator==(const NodePtr& other) const {
	return node == other.node;
}

inline bool NodePtr::operator!=(const NodePtr& other) const {
	return !(*this == other);
}



inline NodeView::NodeView() {
	
}

inline NodeView::NodeView(NodePtr node, IHitCube cube): NodePtr(node), IHitCube(cube) {
	
}

inline NodeView::NodeView(NodePtr node, ivec3 position, int scale): NodePtr(node), IHitCube(position, scale) {
	
}




inline Block* BlockView::operator->() {
	return block();
}

inline NodeView BlockView::nodeview() const {
	return *this;
}




#endif
