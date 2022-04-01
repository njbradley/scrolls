#include "game.h"
#include "graphics.h"
#include "rendering.h"
#include "blocks.h"
#include "blockiter.h"
#include "terrain.h"

#include <set>
#include <sstream>

DEFINE_PLUGIN(Game);


EXPORT_PLUGIN(SingleGame);

const int worldsize = 128;

// From the chunk we are currently in or the position we are in?
const int renderdistance = 1; // Initially 3x3x3
const int chunks = pow(2*(renderdistance + 1), 3);

SingleGame::SingleGame() {
	graphics = GraphicsContext::plugnew();
	renderer = Renderer::plugnew();
	controls = Controls::plugnew();

	generatedWorld.reserve(chunks);


	// Simple loop first.
	// TODO: OPTIMIZE LOOP MAYBE?
	int x = -2*renderdistance;
	int y = -2*renderdistance;
	int z = -2*renderdistance;
	for (int i = 0; i < chunks; i++) {
		// cout << "blah" << endl;
		generatedWorld.push_back(BlockContainer(ivec3(x, y, z)*worldsize, worldsize));
		if (x == 2*renderdistance) {
			if (z == 2*renderdistance) {
				if (y == 2*renderdistance) {
					// Done
				} else {
					y++;
					x = -2*renderdistance;
					z = -2*renderdistance;
				}
			} else {
				z++;
				x = -2*renderdistance;
			}
		} else {
			x++;
		}
	}

}

SingleGame::~SingleGame() {
	plugdelete(graphics);
	plugdelete(renderer);
	plugdelete(controls);
}

void recurse2(NodeView node, std::set<string>& poses) {
	if (node.continues()) {
		for (int i = 0; i < 8; i ++) {
			recurse2(node.child(i), poses);
		}
	} else {
		std::stringstream ss;
		ss << node.globalpos << ' ' << node.scale;
		if (poses.count(ss.str()) != 0) {
			cout << "DUP recurse2: " << ss.str() << endl;
		} else {
			poses.emplace(ss.str());
		}
	}
}

void recurse(Node* node, ivec3 globalpos, int scale, std::set<string>& poses) {
	if (node->flags & Block::CONTINUES_FLAG) {
		for (int i = 0; i < 8; i ++) {
			recurse(&node->children[i], globalpos + ivec3(NodeIndex(i)) * (scale/2), scale/2, poses);
		}
	} else {
		std::stringstream ss;
		ss << globalpos << ' ' << scale;
		if (poses.count(ss.str()) != 0) {
			cout << "DUP recurse: " << ss.str() << endl;
		} else {
			poses.emplace(ss.str());
		}
	}
}

void SingleGame::setup_gameloop() {
	
	cout << "starting test " << BDIMS << endl;
	
	double start = getTime();
	TerrainGenerator* gen = TerrainGenerator::plugnew(12345);
	for (BlockContainer& bc : generatedWorld) {
		gen->generate_chunk(bc.rootview());
	}
	
	cout << gen->get_height(ivec3(0,0,0)) << endl;
	cout << getTime() - start << " Time terrain " << endl;
	
	start = getTime();
	for (BlockContainer& bc : generatedWorld) {
		renderer->render(bc.rootview(), graphics->blockbuf);
	}
	cout << getTime() - start << " Time render " << endl;
	
	start = getTime();
	int num = 0;

	for (BlockContainer& bc : generatedWorld ) {
		for (BlockView view : BlockIterable<BlockIter>(bc.rootview())) {
			num ++;
		}
	}
	cout << getTime() - start << " Time iter (num blocks): " << num << endl;
	
	spectator.controller = controls;
	graphics->set_camera(&spectator.position, &spectator.angle);
}

void SingleGame::timestep() {
	static double last_time = getTime();
	double cur_time = getTime();
	double deltatime = cur_time - last_time;
	last_time = cur_time;
	
	spectator.timestep(cur_time, deltatime);
	graphics->viewbox->timestep(cur_time, deltatime);
	graphics->swap();
	playing = !controls->key_pressed('Q');
	// cout << spectator.position << ' ' << spectator.angle.x << ',' << spectator.angle.y << endl;
}
