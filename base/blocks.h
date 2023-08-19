#ifndef SCROLLS_BLOCKS
#define SCROLLS_BLOCKS

#include "common.h"

#include "physics.h"
#include "memory.h"
#include "graphics.h"

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
with raw Node* pointers, instead use the NodePtr and similar
classes, which wrap a Node* pointer with helpful functions and
bookeeping of size and position, as well as keeping track of
resources
*/
	
struct Block {
	BlockData* type = nullptr;
	RenderIndex renderindex;
	uint8 sunlight = 0;
	uint8 blocklight = 0;
	
	enum : uint32 {
		PROPOGATING_FLAGS = 0xffff0000,
		STRUCTURE_FLAGS = 0x000000ff,
		RENDER_FLAG = 0x00010000,
		GENERATION_FLAG = 0x00020000,
		CHILDREN_FLAG = 0x00000001,
		PARENT_FLAG = 0x00000002,
		FREENODE_FLAG = 0x00000004,
		ENTITY_FLAG = 0x00000008
	};
	
	Block();
	Block(BlockData* newtype);
	Block(const Block& other);
};

struct Node {
	union {
		Node* parent = nullptr;
		BlockContainer* container;
	};
	union {
		Node* children = nullptr;
		Block* block;
	};
	FreeNode* freechild = nullptr;
	uint32 flags = 0;
	uint8 max_depth = 0;
	uint8 last_pix = 0;
};

struct FreeNode : Node {
	FreeNode* next = nullptr;
	quat rotation;
	vec3 offset;
	
	FreeNode();
};




// struct that represents an index into
// a inner node
// can be used to convert between a position
// in child space (ie 0,1,0) to an index
// in the children array (ie 2)
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


struct NodeChildrenIter {
	struct iterator {
		Node* curnode;
		
		iterator& operator++();
		Node* operator*();
		bool operator!=(iterator other);
	};
	
	struct const_iterator {
		const Node* curnode;
		
		const_iterator& operator++();
		const Node* operator*();
		bool operator!=(const_iterator other);
	};
	
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
};


// Class that represents a pointer to a Node in an octree.
// Contains no computed values (ie size or position) that
// NodeView and similar have. Use this class when you don't
// need any of these.

class NodePtr {
public:
	
	NodePtr();
	NodePtr(Node* node);
	
	// whether this view points to a real node
	// default constructed NodeViews will return false,
	// and some methods that can fail (get_global, child)
	// will return an instance that is not valid on failure
	bool isvalid() const;
	// whether the node has a block (accessed by block())
	// hasblock() and haschildren() cannot both be true
	bool hasblock() const;
	// whether the node has children (accesed by child())
	bool haschildren() const;
	bool hassiblings() const;
	bool hasparent() const;
	bool hascontainer() const;
	bool isfreenode() const;
	bool hasfreechild() const;
	bool hasfreesibling() const;
	
	// the index where this node is in the parent node
	// ie: node.parent().child(node.parentindex()) == node
	NodeIndex parentindex() const;
	int max_depth() const;
	
	// the block stored at this node
	// warning: if no block is stored, a possibly random pointer will
	// be returned, so be sure to access this only when sure there
	// is a block (when hasblock() is true)
	Block* block();
	const Block* block() const;
	
	BlockContainer* container();
	const BlockContainer* container() const;
	FreeNode* freenode();
	const FreeNode* freenode() const;
	
	// turns the current view into an invaid instance
	// basically the same as node = NodeView()
	void invalidate();
	
	// returns a new view to the parent node
	NodePtr parent() const;
	NodePtr sibling(NodeIndex index) const;
	// returns a new view to the child at the given index.
	NodePtr child(NodeIndex index) const;
	NodePtr freechild() const;
	NodePtr freesibling() const;
	
	void set_last_pix(BlockData* blockval);
	NodePtr last_pix() const;
	
	// turns a leaf node into an inner node, and
	// creates 8 child nodes
	void split();
	// turns an inner node into a leaf node, deleting
	// all children previously on the node
	void join();
	void subdivide();
	
	// if the nodeptr currently points to a freenode, then
	// it is removed, and the pointer is no longer valid
	void remove_freechild();
	// adds a freechild to the current node
	void add_freechild(vec3 offset, quat rotation);
	
	// these methods read/write/modify the flags set on nodes,
	// where flag is a bit mask. the flags are defined in the
	// block class,
	uint32 test_flag(uint32 flag) const;
	void set_flag(uint32 flag);
	void reset_flag(uint32 flag);
	void set_all_flags(uint32 flag);
	
	// this method should be called whenever the structure or value of
	// a node is changed
	void on_change();
	void update_depth();
	void update_depth(uint8 new_depth);

	// returns a short string with the status of the node, abbreviated
	string status_str() const;
	
	// modify the block that this node holds.
	void set_block(Block* block);
	Block* swap_block(Block* block);
	
	// swaps the tree at the current node with the tree pointed to
	// by other
	void swap_tree(NodePtr other);
	// deletes the current tree and copies the tree pointed to
	// by other onto this node
	void copy_tree(NodePtr other);
	
	template <template <typename> typename NodeIterT, typename ... Args>
	NodeIterable<NodeIterT<NodePtr>> iter(Args ... args);
	NodeIterable<ChildIter<NodePtr>> children();
	
	bool operator==(const NodePtr& other) const;
	bool operator!=(const NodePtr& other) const;
	
	friend ostream& operator<<(ostream& out, NodePtr node);
protected:
	Node* node = nullptr;
	
	void copy_tree(Node* src, Node* dest);
	void del_tree(Node* node);
	void update_child(Node* child);
	
	NodeChildrenIter childreniter();
	const NodeChildrenIter childreniter() const;
	
	template <typename NodePtrT>
	friend class NodeIter;
	template <typename NodePtrT>
	friend class ChildIter;
};




// class that allows reading/modifying
// of the octree
// A nodeview points to a position on the block tree, while
// also keeping track of the current position and size of the
// node pointed to.
// Use this class when a NodePtr wouldn't do (you need position),
// But you don't need to know the true global position of a block
// 
class NodeView : public NodePtr, public IHitCube {
public:
	
	NodeView();
	NodeView(NodePtr node, IHitCube cube);
	NodeView(NodePtr node, ivec3 gpos, int nscale);
	NodeView(NodePtr node, ivec3 gpos, int nscale, RefCounted<NodeView>* hparent);
	// explicit NodeView(NodePtr node)
	
	// see comments in NodePtr
	NodeView parent() const;
	NodeView sibling(NodeIndex index) const;
	NodeView child(NodeIndex index) const;
	NodeView freechild() const;
	NodeView freesibling() const;
	
	// returns a view of the block at the given position
	// if the position is outside of the root node of the tree,
	// an invalid view is returned.
	// if the position is inside the root node but there isnt
	// a node with the given scale, the smallest node will be returned
	// this means passing 1 as scale guarantees you will recieve a leaf node
	NodeView get_global(IHitCube goalbox);
	NodeView get_global(ivec3 pos, int scale);
	
	// returns the smallest scale of any child block below this node
	int min_scale() const;
	
	template <template <typename> typename NodeIterT, typename ... Args>
	NodeIterable<NodeIterT<NodeView>> iter(Args ... args);
	NodeIterable<ChildIter<NodeView>> children();
	
	friend ostream& operator<<(ostream& out, const NodeView& node);
protected:
	RefCounter<NodeView> highparent;
};

// class FreeNodeView : public NodeView, public HitCube {
class FreeNodeView : public NodePtr, public HitCube {
public:
	ivec3 localpos;
	
	FreeNodeView();
	explicit FreeNodeView(const NodeView& node);
	FreeNodeView(NodePtr nodeptr, ivec3 lpos, int scale, RefCounted<FreeNodeView>* hparent);
	
	FreeNodeView parent() const;
	FreeNodeView sibling(NodeIndex index) const;
	FreeNodeView child(NodeIndex index) const;
	FreeNodeView freechild() const;
	FreeNodeView freesibling() const;
	
	explicit operator NodeView() const;
	
	template <template <typename> typename NodeIterT, typename ... Args>
	NodeIterable<NodeIterT<FreeNodeView>> iter(Args ... args);
	NodeIterable<ChildIter<FreeNodeView>> children();
	
	friend ostream& operator<<(ostream& out, const FreeNodeView& node);
protected:
	void recalculate_position();
	
	RefCounter<FreeNodeView> highparent;
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
	// using NodeView::step_down;
	// using NodeView::step_up;
	// using NodeView::step_side;
	// using NodeView::moveto;
	
	using NodeView::join;
	using NodeView::split;
};


// this is the class that allocates and owns an octree
// and all nodes
class BlockContainer : public NodeView {
public:
	BlockContainer(ivec3 pos, int scale);
	BlockContainer(const BlockContainer& other);
	BlockContainer(BlockContainer&& other);
	~BlockContainer();
	
	BlockContainer& operator=(BlockContainer other);
	
	void swap(BlockContainer& other);
	
	virtual BlockContainer* find_neighbor(ivec3 pos, int goalscale);
	NodeView get_global(ivec3 pos, int goal_scale);
};



















// INLINE FUNCTIONS

inline Block::Block() {
	
}

inline Block::Block(BlockData* newtype): type(newtype) {
	
}

inline Block::Block(const Block& other): type(other.type) {
	
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


inline FreeNode::FreeNode() {
	flags = Block::FREENODE_FLAG;
}











inline NodePtr::NodePtr(): node(nullptr) {
	
}

inline NodePtr::NodePtr(Node* nnode): node(nnode) {
	
}

inline bool NodePtr::isvalid() const {
	return node != nullptr;
}

inline bool NodePtr::hasblock() const { ASSERT(isvalid());
	return !haschildren() and node->block != nullptr;
}

inline bool NodePtr::haschildren() const { ASSERT(isvalid());
	return node->flags & Block::CHILDREN_FLAG;
}

inline bool NodePtr::hassiblings() const { ASSERT(isvalid());
	return (node->flags & Block::PARENT_FLAG) and !(node->flags & Block::FREENODE_FLAG);
}

inline bool NodePtr::hasfreechild() const { ASSERT(isvalid());
	return node->freechild != nullptr;
}

inline bool NodePtr::hasparent() const { ASSERT(isvalid());
	return node->flags & Block::PARENT_FLAG;
}

inline bool NodePtr::hascontainer() const { ASSERT(isvalid());
	return !hasparent() and !isfreenode();
}

inline bool NodePtr::isfreenode() const { ASSERT(isvalid());
	return node->flags & Block::FREENODE_FLAG;
}

inline bool NodePtr::hasfreesibling() const { ASSERT(isvalid());
	return isfreenode() and freenode()->next != nullptr;
}

inline uint32 NodePtr::test_flag(uint32 flag) const { ASSERT(isvalid());
	return node->flags & flag;
}

inline NodeIndex NodePtr::parentindex() const { ASSERT(isvalid() and !isfreenode());
	return node - node->parent->children;
}

inline int NodePtr::max_depth() const { ASSERT(isvalid());
	return node->max_depth;
}

inline Block* NodePtr::block() { ASSERT(isvalid() and hasblock());
	return node->block;
}

inline const Block* NodePtr::block() const { ASSERT(isvalid() and hasblock());
	return node->block;
}

inline BlockContainer* NodePtr::container() { ASSERT(isvalid() and hascontainer());
	return node->container;
}

inline const BlockContainer* NodePtr::container() const { ASSERT(isvalid() and hascontainer());
	return node->container;
}

inline FreeNode* NodePtr::freenode() { ASSERT(isfreenode());
	return (FreeNode*) node;
}

inline const FreeNode* NodePtr::freenode() const { ASSERT(isfreenode());
	return (const FreeNode*) node;
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

inline ostream& operator<<(ostream& out, NodePtr node) {
	return out << "NodePtr(" << node.status_str() << ' ' << node.node << ")";
}

template <template <typename> typename NodeIterT, typename ... Args>
NodeIterable<NodeIterT<NodePtr>> NodePtr::iter(Args ... args) {
	return {*this, args...};
}

inline NodeView::NodeView() {
	
}

inline NodeView::NodeView(NodePtr node, IHitCube cube): NodePtr(node), IHitCube(cube) {
	
}

inline NodeView::NodeView(NodePtr node, ivec3 position, int scale): NodePtr(node), IHitCube(position, scale) {
	
}

inline NodeView::NodeView(NodePtr node, ivec3 position, int scale, RefCounted<NodeView>* hparent):
NodePtr(node), IHitCube(position, scale), highparent(hparent) {
	
}

inline ostream& operator<<(ostream& out, const NodeView& node) {
	return out << "NodeView(" << node.status_str() << ' ' << node.position << ' ' << node.scale << ")";
}


inline FreeNodeView::FreeNodeView() {
	
}

inline ostream& operator<<(ostream& out, const FreeNodeView& node) {
	return out << "NodeView(" << node.status_str() << ' ' << node.position << ' ' << node.scale << ' ' << node.rotation << ")";
}

// inline FreeNodeView::FreeNodeView(const NodeView& nodeview): NodeView(nodeview) {
//
// }

template <template <typename> typename NodeIterT, typename ... Args>
NodeIterable<NodeIterT<NodeView>> NodeView::iter(Args ... args) {
	return {*this, args...};
}

template <template <typename> typename NodeIterT, typename ... Args>
NodeIterable<NodeIterT<FreeNodeView>> FreeNodeView::iter(Args ... args) {
	return {*this, args...};
}

inline Block* BlockView::operator->() {
	return block();
}

inline NodeView BlockView::nodeview() const {
	return *this;
}




#endif
