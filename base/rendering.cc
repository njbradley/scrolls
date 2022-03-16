#include "rendering.h"
#include "blocks.h"
#include "blockiter.h"
#include "graphics.h"

DEFINE_PLUGIN(Renderer);

EXPORT_PLUGIN(DefaultRenderer);

bool DefaultRenderer::render(NodeView mainblock, RenderBuf* renderbuf) {
	bool changed = false;
	
	// for (BlockView block : BlockIterable<FlagBlockIter>(mainblock, Block::RENDER_FLAG)) {
	for (NodeView& node : BlockIterable<FlagNodeIter>(mainblock, Block::RENDER_FLAG)) {
		node.reset_flag(Block::RENDER_FLAG);
		if (node.continues()) continue;
		BlockView block = node;
		// continue;
		if (block->value != 0) {
			RenderData data;
			bool visible = false;
			
			data.posdata.pos = block.globalpos;
			data.posdata.scale = block.scale;
			
			for (Direction dir : Direction::all) {
				data.facedata.faces[dir].texture = 0;
				NodeView sidenode = block.get_global(block.globalpos + ivec3(dir) * block.scale, block.scale);
				for (BlockView sideblock : BlockIterable<DirBlockIter>(sidenode, -ivec3(dir))) {
					if (sideblock->value == 0) {
						data.facedata.faces[dir].texture = 1;
						visible = true;
					}
				}
			}
			
			// for (Direction dir : Direction::all) {
				// data.facedata.faces[dir].texture = visible;
			// }
			
			visible = true;
			if (visible) {
				if (block->renderindex != -1) {
					renderbuf->edit(block->renderindex, data);
				} else {
					cout << "adding block " << endl;
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
