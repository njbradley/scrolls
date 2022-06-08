#ifndef BASE_FILEFORMAT
#define BASE_FILEFORMAT

#include "common.h"
#include "plugins.h"

#include "blocks.h"
#include <fstream>

class BlockFileSystem {
	BASE_PLUGIN(BlockFileSystem, (string dirpath));
public:
	string dirpath;
	
	BlockFileSystem(string path);
	virtual ~BlockFileSystem();
	
	virtual void to_file(NodeView node) = 0;
	virtual bool from_file(NodeView node) = 0;
	
protected:
	BlockFileFormat* fileformat;
};


class IndividualFileSystem : public BlockFileSystem {
	PLUGIN(IndividualFileSystem);
public:
	
	using BlockFileSystem::BlockFileSystem;
	virtual void to_file(NodeView node);
	virtual bool from_file(NodeView node);
};









class BlockFileFormat {
	BASE_PLUGIN(BlockFileFormat, ());
public:
	virtual ~BlockFileFormat() {}
	virtual void from_file(NodePtr node, istream& ifile) = 0;
	virtual void to_file(NodePtr node, ostream& ofile) = 0;
};

class SequentialFileFormat : public BlockFileFormat {
	PLUGIN(SequentialFileFormat);
public:
	virtual void from_file(NodePtr node, istream& ifile);
	virtual void to_file(NodePtr node, ostream& ofile);
};

#endif
