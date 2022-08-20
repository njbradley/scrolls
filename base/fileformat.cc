#include "fileformat.h"

#include "blocks.h"
#include "blockiter.h"
#include "blockdata.h"

#include <sstream>
#include <fstream>

DEFINE_PLUGIN(BlockFileSystem);

BlockFileSystem::BlockFileSystem(string dir): dirpath(dir) {
	fileformat = BlockFileFormat::plugnew();
}

BlockFileSystem::~BlockFileSystem() {
	plugdelete(fileformat);
}


EXPORT_PLUGIN(IndividualFileSystem);

bool IndividualFileSystem::from_file(NodeView node) {
	std::ostringstream path;
	path << dirpath << node.position.x << "x" << node.position.y << "y" << node.position.z << "z" << node.scale << ".txt";
	std::ifstream ifile (path.str(), std::ios::binary);
	if (ifile.good()) {
		fileformat->from_file(node, ifile);
		return true;
	}
	return false;
}

void IndividualFileSystem::to_file(NodeView node) {
	std::ostringstream path;
	path << dirpath << node.position.x << "x" << node.position.y << "y" << node.position.z << "z" << node.scale << ".txt";
	std::ofstream ofile (path.str(), std::ios::binary);
	fileformat->to_file(node, ofile);
}



DEFINE_PLUGIN(BlockFileFormat);


EXPORT_PLUGIN(SequentialFileFormat);

void SequentialFileFormat::from_file(NodePtr node, istream& ifile) {
	vector<BlockData*> blockarr;
	for (BlockData* data : BlockData::allblocks) {
		if (data->id > blockarr.size()) {
			blockarr.resize(data->id, nullptr);
		}
		blockarr[data->id-1] = data;
	}
	for (NodePtr curnode : node.iter<NodeIter>()) {
		if (ifile.peek() == '{') {
			ifile.get();
			curnode.split();
		} else if (ifile.peek() == '~') {
			ifile.get();
			curnode.set_block(nullptr);
		} else {
			int index = ifile.get();
			curnode.set_block(new Block(index == 0 ? nullptr : blockarr[index-1]));
		}
	}
}

void SequentialFileFormat::to_file(NodePtr node, ostream& ofile) {
	for (NodePtr curnode : node.iter<NodeIter>()) {
		if (curnode.haschildren()) {
			ofile.put('{');
		} else if (curnode.hasblock()) {
			ofile.put(curnode.block()->type->id);
		} else {
			ofile.put('~');
		}
	}
}
