#ifndef BLOCKITER_H
#define BLOCKITER_H

#include "common.h"
#include "blocks.h"

template <typename Iterator>
class BlockIterable {
public:
	Iterator iter;
	template <typename ... Args>
	BlockIterable(Args&& ... args): iter(std::forward<Args>(args)...) { }
	
	Iterator begin() { return iter; }
	Iterator end() { Iterator enditer = iter; enditer.invalidate(); return enditer; }
};

class BlockIter : public NodeIter {
public:
	using NodeIter::NodeIter;
protected:
	virtual bool valid_node() { return !continues(); }
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
	virtual bool valid_node() { return !continues(); }
};


class FlagNodeIter : public NodeIter {
public:
	uint flag;
	FlagNodeIter(const NodeView& node, uint nflag): NodeIter(node), flag(nflag) { }
protected:
	virtual bool valid_tree();
};

class FlagBlockIter : public FlagNodeIter {
public:
	using FlagNodeIter::FlagNodeIter;
protected:
	virtual bool valid_node() { return !continues(); }
};




#endif
