#include "rendering.h"
#include "arrblocks.h"
#include "arrblockiter.h"
#include "graphics.h"

DEFINE_PLUGIN(Renderer);

EXPORT_PLUGIN(DefaultRenderer);

bool DefaultRenderer::render(BlockContainer& container, RenderBuf* renderbuf) {
	bool changed = false;
	
	// for (BlockView block : BlockIterable<FlagBlockIter>(mainblock, Block::RENDER_FLAG)) {
	for (BlockView& block : BlockIterable<BlockIter>(container)) {
		// continue;
		if (block->value != 0) {
			RenderData data;
			bool visible = false;
			
			// cout << "BEGIN BLOCK " << block.container.globalPos << endl;
			
			data.posdata.pos = vec3(block.pos);
			data.posdata.scale = 1;
			
			for (Direction dir : Direction::all) {
				// cout << __LINE__ << endl;
				data.facedata.faces[dir].texture = 0;
				data.facedata.faces[dir].other = 0;
				data.facedata.faces[dir].stuff = 0;

				BlockView sideblock = BlockView(block);
				// cout << "  going to " << block.globalpos + ivec3(dir) * block.scale << endl;
				// cout << "  SIDENODE " << sidenode.globalpos << ' ' << sidenode.scale << endl;
				if (sideblock.moveto(block.pos + ivec3(dir))) {

					// cout << "    SIDEBLOCK " << sideblock.container.globalPos << endl;
					if (sideblock->value == 0) {
						data.facedata.faces[dir].texture = block->value;
						data.facedata.faces[dir].sunlight = 0x0f;
						data.facedata.faces[dir].blocklight = 0x00;
						data.facedata.faces[dir].other = 0;
						data.facedata.faces[dir].stuff = 0;
						data.facearr[int(dir)*2+1] = 10;
						visible = true;
					}
				
				}
				// else {
				// 	data.facedata.faces[dir].texture = block->value;
				// 	visible = true;
				// }
					
			}
			
			// for (Direction dir : Direction::all) {
			// 	cout << data.facearr[int(dir)*2] << ' ';
			// } cout << endl;
			
			// visible = true;
			if (visible) {
				// cout << "bdla" << endl;
				if (block->renderindex != -1) {
					renderbuf->edit(block->renderindex, data);
				} else {
					block->renderindex = renderbuf->add(data);
				}
			} else if (block->renderindex != -1) {
				renderbuf->del(block->renderindex);
				block->renderindex = -1;
			}
		} else if (block->renderindex != -1) {
			renderbuf->del(block->renderindex);
			block->renderindex = -1;
		}
		changed = true;
	}
	return changed;
}
