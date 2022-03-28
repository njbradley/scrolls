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

SingleGame::SingleGame(): world(ivec3(-worldsize/2,0,-worldsize/2), worldsize) {
	graphics = GraphicsContext::plugnew();
	renderer = Renderer::plugnew();
	controls = Controls::plugnew();
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
	gen->generate_chunk(world.rootview());
	cout << gen->get_height(ivec3(0,0,0)) << endl;
	cout << getTime() - start << " Time terrain " << endl;
	
	start = getTime();
	renderer->render(world.rootview(), graphics->blockbuf);
	cout << getTime() - start << " Time render " << endl;
	
	start = getTime();
	int num = 0;
	for (BlockView view : BlockIterable<BlockIter>(world.rootview())) {
		num ++;
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
