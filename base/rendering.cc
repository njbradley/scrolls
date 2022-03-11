#include "rendering.h"
#include "blocks.h"
#include "blockiter.h"
#include "graphics.h"

DEFINE_PLUGIN(Renderer);

EXPORT_PLUGIN(DefaultRenderer);

bool DefaultRenderer::render(BlockView mainblock, RenderBuf* renderbuf) {
	bool changed = false;
	
	FlagBlockIter iter (mainblock, Block::RENDER_FLAG);
	
	for (BlockView block : BlockIterable<FlagBlockIter>(mainblock, Block::RENDER_FLAG)) {
		
		if (block->value != 0) {
			RenderData data;
			
			for (Direction dir : Direction::all) {
				data.facedata.faces[dir].texture = 0;
				NodeView sidenode = block.get_global(block.globalpos + ivec3(dir) * block.scale, block.scale);
				for (BlockView sideblock : BlockIterable<DirBlockIter>(sidenode, -ivec3(dir))) {
					if (sideblock->value == 0) {
						data.facedata.faces[dir].texture = 1;
					}
				}
			}
			
			if (block->renderindex != -1) {
				renderbuf->edit(block->renderindex, data);
			} else {
				block->renderindex = renderbuf->add(data);
			}
		} else if (block->renderindex != -1) {
			renderbuf->del(block->renderindex);
			block->renderindex = -1;
		}
		changed = true;
	}
	return changed;
}
