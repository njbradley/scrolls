#ifndef BLOCKITER_H
#define BLOCKITER_H

#include "common.h"
#include "blocks.h"

// template <typename NodePtrT, typename Inherited = NodeIter<NodePtrT>>
// class NodeIter {
// 	NodePtrT curnode;
// 	int max_scale;
//
// 	NodeIter(NodePtrT& node);
//
// 	virtual NodeIndex increment_func(NodeIndex index);
//
// 	void step_down(NodePtrT node);
// 	void step_side(NodePtrT node);
// 	void get_safe(NodePtrT node);
//
// 	virtual bool valid_tree(NodePtrT node);
// 	virtual bool valid_node(NodePtrT node);
//
// 	bool operator!=(const NodeIter<NodePtrT>& other);
//
// 	Inherited begin() const;
// 	Inherited end() const;
// };













template <typename Iterator>
class BlockIterable {
public:
	Iterator iter;
	template <typename ... Args>
	BlockIterable(Args&& ... args): iter(std::forward<Args>(args)...) { iter.get_safe(); }
	
	Iterator begin() { return iter; }
	Iterator end() { Iterator enditer = iter; enditer.invalidate(); return enditer; }
};

class BlockIter : public NodeIter {
public:
	using NodeIter::NodeIter;
protected:
	virtual bool valid_node() const { return !haschildren(); }
};

class DirNodeIter : public NodeIter {
public:
	Direction dir;
	DirNodeIter(const NodeView& node, Direction ndir): NodeIter(node), dir(ndir) { }
	
  virtual NodeIndex startpos() { return (ivec3(dir)+1)/2 * (BDIMS-1); };
  virtual NodeIndex endpos()  { return (1-(1-ivec3(dir))/2) * (BDIMS-1); }
	virtual NodeIndex increment_func(NodeIndex nodepos);
};

class DirBlockIter : public DirNodeIter {
public:
	using DirNodeIter::DirNodeIter;
protected:
	virtual bool valid_node() const { return !haschildren(); }
};


class FlagNodeIter : public NodeIter {
public:
	uint flag;
	FlagNodeIter(const NodeView& node, uint nflag): NodeIter(node), flag(nflag) { }
protected:
	virtual bool valid_tree() const;
};

class FlagBlockIter : public FlagNodeIter {
public:
	using FlagNodeIter::FlagNodeIter;
protected:
	virtual bool valid_node() const { return !haschildren(); }
};




#endif
