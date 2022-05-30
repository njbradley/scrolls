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
		if (node.haschildren()) continue;
		BlockView block = node;
		// continue;
		if (block->value != 0) {
			RenderData data;
			bool visible = false;
			
			// cout << "BEGIN BLOCK " << block.position << ' ' << block.scale << endl;
			
			data.posdata.pos = vec3(block.position) + float(block.scale) / 2;
			data.posdata.scale = block.scale;
			
			for (Direction dir : Direction::all) {
				data.facedata.faces[dir].texture = 0;
				data.facedata.faces[dir].other = 0;
				data.facedata.faces[dir].stuff = 0;
				NodeView sidenode = block.get_global(block.position + ivec3(dir) * block.scale, block.scale);
				// cout << "  going to " << block.position + ivec3(dir) * block.scale << endl;
				// cout << "  SIDENODE " << sidenode.position << ' ' << sidenode.scale << endl;
				if (sidenode.isvalid()) {
					for (BlockView sideblock : BlockIterable<DirBlockIter>(sidenode, -ivec3(dir))) {
						// cout << "    SIDEBLOCK " << sideblock.position << ' ' << sideblock.scale << endl;
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
				}
				// else {
				// 	data.facedata.faces[dir].texture = block->value;
				// 	visible = true;
				// }
					
			}
			
			// for (Direction dir : Direction::all) {
			// 	cout << data.facearr[int(dir)*2] << ' ';
			// } cout << endl;
			
			if (visible) {
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
