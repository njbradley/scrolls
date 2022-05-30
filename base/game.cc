#include "game.h"
#include "graphics.h"
#include "rendering.h"
#include "blocks.h"
#include "blockiter.h"
#include "terrain.h"
#include "debug.h"
#include "entity.h"

#include <set>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

DEFINE_PLUGIN(Game);


EXPORT_PLUGIN(SingleGame);

const int worldsize = 32;

const int renderdistance = 2;
const int chunks = 8;

const bool overwrite_saves = true;




Chunk::Chunk(SingleGame* newgame, ivec3 pos, int scale): game(newgame), BlockContainer(pos,scale) {
  
}

BlockContainer* Chunk::find_neighbor(ivec3 pos, int goalscale) {
  for (Chunk& chunk : game->generatedWorld) {
    if (chunk.contains(IHitCube(pos, goalscale))) {
      return &chunk;
    }
  }
  return nullptr;
}






SingleGame::SingleGame() {
	graphics = GraphicsContext::plugnew();
	renderer = Renderer::plugnew();
	controls = Controls::plugnew();

	generatedWorld.reserve(chunks);


	// Simple loop first.
	// TODO: OPTIMIZE LOOP MAYBE?
	int x = -1;
	int y = -1;
	int z = -1;
	for (int i = 0; i < chunks; i++) {
		cout << "blah: " << i << endl;
		cout << "x: " << x << " y: " << y << " z: " << z << endl;
    // generatedWorld.emplace_back(ivec3(x, y, z)*worldsize, worldsize);
		generatedWorld.push_back(Chunk(this, ivec3(x, y, z)*worldsize, worldsize));
		if (x == 0) {
			if (z == 0) {
				if (y == 0) {
					// Done
				} else {
					y++;
					x = -1;
					z = -1;
				}
			} else {
				z++;
				x = -1;
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


void SingleGame::setup_gameloop() {
	
	cout << "starting test " << BDIMS << endl;
	
	double start = getTime();
	TerrainGenerator* gen = TerrainGenerator::plugnew(12345);
  cout << gen->get_height(ivec3(0,0,0)) << " get_height" << endl;
  
	for (BlockContainer& bc : generatedWorld) {
		std::ostringstream oss;
		oss << "./world/chunks/" << bc.position.x << "x" << bc.position.y << "y" << bc.position.z << "z" << worldsize << ".txt";
		struct stat buf;
		// If the file does not exist, create terrain.
		// Otherwise, read from file.
		if (stat(oss.str().c_str(), &buf) != 0 or overwrite_saves) {
			gen->generate_chunk(bc.root());
			std::ofstream outfile(oss.str(), std::ios::binary);
			bc.to_file(outfile);
			outfile.close();
		} else {
			std::ifstream t(oss.str().c_str(), std::ios::binary);
			bc.from_file(t);
		}
	}
  
	cout << getTime() - start << " Time terrain " << endl;
	start = getTime();
  
	for (BlockContainer& bc : generatedWorld) {
		renderer->render(bc.root(), graphics->blockbuf);
	}
  
	cout << getTime() - start << " Time render " << endl;
	
	start = getTime();
	int num = 0;

	for (BlockContainer& bc : generatedWorld ) {
		for (BlockView view : BlockIterable<BlockIter>(bc.root())) {
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
	
  std::stringstream debugstr;
  debugstr << "FPS: " << 1 / deltatime << endl;
  debugstr << "spectatorpos: " << spectator.position << endl;
  debugstr << "abcdefghijklmnopqrstuvwxyz" << endl;
  debugstr << "1234567890 [({<()>})]" << endl;
  debuglines->clear();
  debuglines->draw(vec2(-0.99, 0.95), debugstr.str());
  
	spectator.timestep(cur_time, deltatime);
	graphics->viewbox->timestep(cur_time, deltatime);
	graphics->swap();
	playing = !controls->key_pressed('Q');
	// cout << spectator.position << ' ' << spectator.angle.x << ',' << spectator.angle.y << endl;
}
