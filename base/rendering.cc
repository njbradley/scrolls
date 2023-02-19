#include "rendering.h"
#include "blocks.h"
#include "blockiter.h"
#include "graphics.h"
#include "blockdata.h"

DEFINE_PLUGIN(Renderer);

EXPORT_PLUGIN(DefaultRenderer);

void DefaultRenderer::derender(NodePtr nv, RenderBuf* renderbuf) {
	for (NodePtr block : NodeIterable<BlockIter<NodePtr>>(nv)) {
		block.block()->renderindex.clear();
	}
}

bool DefaultRenderer::render(NodeView mainblock, RenderBuf* renderbuf) {
	bool changed = false;
	
	// for (BlockView block : NodeIterable<FlagBlockIter>(mainblock, Block::RENDER_FLAG)) {
	// for (NodeView& node : mainblock.iter<FlagNodeIter>(Block::RENDER_FLAG)) {
	for (FreeNodeView& node : FreeNodeView(mainblock).iter<FlagNodeIter>(Block::RENDER_FLAG)) {
		node.reset_flag(Block::RENDER_FLAG);
		if (!node.hasblock()) continue;
		BlockView block = NodeView(node);
		// continue;
		block->renderindex.clear();
		
		if (block->type != nullptr) {
			RenderFace faces[6];
			int num_faces = 0;
			float scale = node.scale/2.0f;
			vec3 center = node.transform_out(vec3(scale));
			
			for (Direction dir : Direction::all) {
				NodeView sidenode = block.get_global(block.position + ivec3(dir) * block.scale, block.scale);
				if (sidenode.isvalid()) {
					for (BlockView sideblock : NodeIterable<DirBlockIter<NodeView>>(sidenode, -ivec3(dir))) {
						if (sideblock->type == nullptr) {
							Direction x_dir = (int(dir) + 1)%6;
							Direction y_dir = (int(dir) + 2)%6;
							if (dir == Direction::POSITIVE_X or dir == Direction::POSITIVE_Z or dir == Direction::NEGATIVE_Y) {
								std::swap(x_dir, y_dir);
							}
							int sunlight = 220;
							if (dir == Direction::POSITIVE_Y) {
								sunlight = 255;
							} else if (dir == Direction::NEGATIVE_Y) {
								sunlight = 200;
							}
							faces[num_faces++] = RenderFace(
								center + vec3(ivec3(dir)) * scale,
								vec3(ivec3(x_dir)) * scale, vec3(ivec3(y_dir)) * scale,
								vec2(block.scale, block.scale),
								sunlight, 0, 
								block->type->textures[dir] + 1
							);
							break;
						}
					}
				}
			}
			
			block->renderindex.clear();
			if (num_faces != 0) {
				renderbuf->add(faces, num_faces, &block->renderindex);
			}
		}
		changed = true;
	}
	return changed;
}
