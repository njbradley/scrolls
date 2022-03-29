#include "game.h"
#include "graphics.h"
#include "rendering.h"
#include "arrblocks.h"
#include "arrblockiter.h"
#include "terrain.h"

#include <set>
#include <sstream>

DEFINE_PLUGIN(Game);


EXPORT_PLUGIN(SingleGame);

const int worldsize = 128;

SingleGame::SingleGame(): world(ivec3(-worldsize/2,0,-worldsize/2)) {
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
		
	double start = getTime();
	// TerrainGenerator* gen = TerrainGenerator::plugnew(12345);
	// gen->generate_chunk(world.rootview());

	for (BlockView& block : BlockIterable<BlockIter>(world)) {
		if (block.pos.y > 5) {
			block->value = 0;
		} else {
			block->value = 1;
		}
	}

	// cout << gen->get_height(ivec3(0,0,0)) << endl;
	cout << getTime() - start << " Time terrain " << endl;
	
	start = getTime();
	renderer->render(world, graphics->blockbuf);
	cout << getTime() - start << " Time render " << endl;
	
	start = getTime();
	int num = 0;
	for (BlockView& view : BlockIterable<BlockIter>(world)) {
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
