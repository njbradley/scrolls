#ifndef BLOCKITER_H
#define BLOCKITER_H

#include "common.h"
#include "blocks.h"

template <typename NodePtrT>
class NodeIter {
public:
	NodeIter(const NodePtrT& node);
	
	NodeIter<NodePtrT> operator++();
	NodePtrT& operator*();
	
	void step_down();
	void step_side();
	void get_safe();
	
	void to_end();

	bool operator!=(const NodeIter<NodePtrT>& other);
	
protected:
	NodePtrT node;
	NodePtr highest_node;
	
	virtual NodeIndex startpos();
  virtual NodeIndex endpos();
	virtual NodeIndex increment_func(NodeIndex index);
	
	virtual bool valid_tree() const;
	virtual bool valid_node() const;
	
	virtual void finish();
};













template <typename Iterator>
class BlockIterable {
public:
	Iterator iter;
	template <typename ... Args>
	BlockIterable(Args&& ... args): iter(std::forward<Args>(args)...) { iter.get_safe(); }
	
	Iterator begin() { return iter; }
	Iterator end() { Iterator enditer = iter; enditer.to_end(); return enditer; }
};

template <typename NodePtrT>
class BlockIter : public NodeIter<NodePtrT> {
public:
	using NodeIter<NodePtrT>::NodeIter;
protected:
	using NodeIter<NodePtrT>::node;
	virtual bool valid_node() const { return !node.haschildren(); }
};

template <typename NodePtrT>
class DirNodeIter : public NodeIter<NodePtrT> {
public:
	Direction dir;
	DirNodeIter(const NodePtrT& node, Direction ndir): NodeIter<NodePtrT>(node), dir(ndir) { }
protected:
  virtual NodeIndex startpos() { return (ivec3(dir)+1)/2 * (BDIMS-1); };
  virtual NodeIndex endpos()  { return (1-(1-ivec3(dir))/2) * (BDIMS-1); }
	virtual NodeIndex increment_func(NodeIndex nodepos);
};

template <typename NodePtrT>
class DirBlockIter : public DirNodeIter<NodePtrT> {
public:
	using DirNodeIter<NodePtrT>::DirNodeIter;
protected:
	using NodeIter<NodePtrT>::node;
	virtual bool valid_node() const { return !node.haschildren(); }
};

template <typename NodePtrT>
class FlagNodeIter : public NodeIter<NodePtrT> {
public:
	uint flag;
	FlagNodeIter(const NodePtrT& node, uint nflag): NodeIter<NodePtrT>(node), flag(nflag) { }
protected:
	using NodeIter<NodePtrT>::node;
	virtual bool valid_tree() const { return node.test_flag(flag); }
};

template <typename NodePtrT>
class FlagBlockIter : public FlagNodeIter<NodePtrT> {
public:
	using FlagNodeIter<NodePtrT>::FlagNodeIter;
protected:
	using NodeIter<NodePtrT>::node;
	virtual bool valid_node() const { return !node.haschildren(); }
};






///// INLINE FUNCTIONS /////////

template <typename NodePtrT>
inline NodeIndex NodeIter<NodePtrT>::startpos() {
	return NodeIndex(0);
}

template <typename NodePtrT>
inline NodeIndex NodeIter<NodePtrT>::endpos() {
	return NodeIndex(BDIMS3-1);
}

template <typename NodePtrT>
inline NodeIndex NodeIter<NodePtrT>::increment_func(NodeIndex pos) {
	return int(pos) + 1;
}

template <typename NodePtrT>
inline NodePtrT& NodeIter<NodePtrT>::operator*() {
	return node;
}

template <typename NodePtrT>
inline bool NodeIter<NodePtrT>::valid_tree() const {
	return true;
}

template <typename NodePtrT>
inline bool NodeIter<NodePtrT>::valid_node() const {
	return true;
}

template <typename NodePtrT>
inline void NodeIter<NodePtrT>::finish() {
	to_end();
}

template <typename NodePtrT>
inline void NodeIter<NodePtrT>::to_end() {
	node.invalidate();
}

template <typename NodePtrT>
inline bool NodeIter<NodePtrT>::operator!=(const NodeIter<NodePtrT>& other) {
	return node != other.node;
}





#endif
