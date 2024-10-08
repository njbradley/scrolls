#ifndef BASE_BLOCKDATA
#define BASE_BLOCKDATA

#include "common.h"
#include "plugins.h"

struct BlockDataParams {
	int id;
    bool visible = true;
    bool transparent = false;
    string texture;
    string textures[6];
};

class BlockData {
    public:
	int id;
    bool visible;
    bool transparent;
    string texture_paths[6];
    int textures[6];
    
    BlockData(BlockDataParams params);
    ~BlockData();

    void set_texture_ids(GraphicsContext* graphics);

	static vector<BlockData*> allblocks;
	static void init(GraphicsContext* graphics);
};


namespace blocktypes {
    extern BlockData air;
	extern BlockData dirt;
	extern BlockData grass;
	extern BlockData stone;
	extern BlockData wood;
	extern BlockData snow;
	extern BlockData leaves;
	extern BlockData lightstone;
	extern BlockData darkstone;
}

#endif
