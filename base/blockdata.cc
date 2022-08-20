#include "blockdata.h"

#include "graphics.h"

#include <algorithm>

BlockData::BlockData(BlockDataParams params) {
    if (params.texture != "") {
        for (int i = 0; i < 6; i ++) {
            texture_paths[i] = params.texture;
        }
    } else {
        for (int i = 0; i < 6; i ++) {
            texture_paths[i] = params.textures[i];
        }
    }
	allblocks.push_back(this);
}

BlockData::~BlockData() {
	vector<BlockData*>::iterator iter = std::find(allblocks.begin(), allblocks.end(), this);
	if (iter != allblocks.end()) {
		allblocks.erase(iter);
	}
}

void BlockData::set_texture_ids(GraphicsContext* graphics) {
    for (int i = 0; i < 6; i ++) {
        textures[i] = graphics->get_texture_id(texture_paths[i]);
    }
}

vector<BlockData*> BlockData::allblocks;

void BlockData::init(GraphicsContext* graphics) {
	for (BlockData* data : allblocks) {
		data->set_texture_ids(graphics);
	}
}

namespace blocktypes {
	BlockData dirt ({.id = 1, .texture = "dirt.bmp"});
	BlockData grass ({.id = 1, .texture = "grass.bmp"});
	BlockData stone ({.id = 1, .texture = "stone.bmp"});
	BlockData wood ({.id = 1, .texture = "wood.bmp"});
};


