#include "game.h"
#include "graphics.h"
#include "rendering.h"
#include "blocks.h"
#include "blockiter.h"
#include "terrain.h"

DEFINE_PLUGIN(Game);


EXPORT_PLUGIN(SingleGame);

SingleGame::SingleGame(): world(ivec3(0,0,0), 128) {
	graphics = GraphicsContext::plugnew();
	renderer = Renderer::plugnew();
	controls = Controls::plugnew();
}

SingleGame::~SingleGame() {
	plugdelete(graphics);
	plugdelete(renderer);
	plugdelete(controls);
}

void SingleGame::setup_gameloop() {
	
	cout << "START " << endl;
	// for (NodeView node : BlockIterable<NodeIter>(world.rootview())) {
	// 	if (node.continues()) continue;
	// 	ivec3 pos = node.globalpos + node.scale/2;
	// 	if (std::abs(pos.x/2 - pos.y) < node.scale/2) {
	// 		node.split();
	// 	} else {
	// 		Block* block = new Block();
	// 		block->value = pos.x/2 > pos.y;
	// 		node.set_block(block);
	// 		// cout << "ended at " << node.scale << endl;
	// 	}
	// }
	
	TerrainGenerator* gen = TerrainGenerator::plugnew(12345);
	gen->generate_chunk(world.rootview());
	cout << gen->get_height(ivec3(0,0,0)) << endl;
	
	cout << "END " << endl;
	
	renderer->render(world.rootview(), graphics->blockbuf);
	cout << "ENDEND" << endl;
	
	spectator.controller = controls;
	graphics->set_camera(&spectator.position, &spectator.angle);
}

void SingleGame::timestep() {
	static double last_time = getTime();
	double cur_time = getTime();
	double deltatime = cur_time - last_time;
	last_time = cur_time;
	
	spectator.timestep(cur_time, deltatime);
	graphics->swap();
	playing = !controls->key_pressed('Q');
	// cout << spectator.position << ' ' << spectator.angle.x << ',' << spectator.angle.y << endl;
}
