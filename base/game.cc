#include "game.h"
#include "graphics.h"
#include "rendering.h"
#include "blocks.h"
#include "blockiter.h"
#include "terrain.h"
#include "threadpool/pool.h"

#include <set>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <thread> 
#include <sys/types.h>
#include <sys/stat.h>

DEFINE_PLUGIN(Game);


EXPORT_PLUGIN(SingleGame);

const int worldsize = 32;

const int renderdistance = 32;
const int chunks = 8;
const int chunkloadingstart = 1;

const bool overwrite_saves = false;


// Pool jobPool(4);



SingleGame::SingleGame() {
	graphics = GraphicsContext::plugnew();
	renderer = Renderer::plugnew();
	controls = Controls::plugnew();

	generatedWorld.reserve(chunks);

 	generator = TerrainGenerator::plugnew(12345);

	// Simple loop first.
	// TODO: OPTIMIZE LOOP MAYBE?
	int x = -chunkloadingstart;
	int y = -chunkloadingstart;
	int z = -chunkloadingstart;
	for (int i = 0; i < chunks; i++) {
		cout << "blah: " << i << endl;
		cout << "x: " << x << " y: " << y << " z: " << z << endl;
    // generatedWorld.emplace_back(ivec3(x, y, z)*worldsize, worldsize);
		generatedWorld.push_back(BlockContainer(ivec3(x, y, z)*worldsize, worldsize));
		if (x == chunkloadingstart) {
			if (z == chunkloadingstart) {
				if (y == chunkloadingstart) {
					// Done
				} else {
					y++;
					x = -chunkloadingstart;
					z = -chunkloadingstart;
				}
			} else {
				z++;
				x = -chunkloadingstart;
			}
		} else {
			x++;
		}
	}

	nick = std::thread(&SingleGame::threadRenderJob, this);


}

SingleGame::~SingleGame() {
	plugdelete(graphics);
	plugdelete(renderer);
	plugdelete(controls);
}

vector<ivec3> chunksInRenderDistance(vec3 playerPos) {
	int x_rem = static_cast<int>(playerPos.x) % renderdistance;
	int y_rem = static_cast<int>(playerPos.y) % renderdistance;
	int z_rem = static_cast<int>(playerPos.z) % renderdistance;

	if (x_rem < 0) {
		x_rem += renderdistance;
	}

	if (y_rem < 0) {
		y_rem += renderdistance;
	}

	if(z_rem < 0) {
		z_rem += renderdistance;
	}
	
	int x = -chunkloadingstart;
	int y = -chunkloadingstart;
	int z = -chunkloadingstart;

	vector<ivec3> newChunks = vector<ivec3>();
	for (int i = 0; i < chunks; i++) {
		newChunks.push_back(ivec3(playerPos.x - x_rem, playerPos.y - y_rem, playerPos.z - z_rem) + (ivec3(x,y,z)*worldsize));
		if (x == chunkloadingstart) {
			if (z == chunkloadingstart) {
				if (y == chunkloadingstart) {
					// Done
				} else {
					y++;
					x = -chunkloadingstart;
					z = -chunkloadingstart;
				}
			} else {
				z++;
				x = -chunkloadingstart;
			}
		} else {
			x++;
		}
	}
	

	return newChunks;
}

void SingleGame::loadOrGenerateTerrain(BlockContainer& bc) {
		std::ostringstream oss;
		oss << "./world/chunks/" << bc.globalpos.x << "x" << bc.globalpos.y << "y" << bc.globalpos.z << "z" << worldsize << ".txt";
		struct stat buf;
		// If the file does not exist, create terrain.
		// Otherwise, read from file.
		if (stat(oss.str().c_str(), &buf) != 0) {
			cout << "GENERATE" << endl;
			generator->generate_chunk(bc.root());
			std::ofstream outfile(oss.str(), std::ios::binary);
			bc.root().to_file(outfile);
			outfile.close();
		} else {
			std::ifstream t(oss.str().c_str(), std::ios::binary);	
			bc.root().from_file(t);
			// cout << "LOAD " << oss.str().c_str() << endl;
			
		}
}


void SingleGame::setup_gameloop() {
	
	cout << "starting test " << BDIMS << endl;
	
	double start = getTime();
	TerrainGenerator* gen = TerrainGenerator::plugnew(12345);
  cout << gen->get_height(ivec3(0,0,0)) << " get_height" << endl;
  
	for (BlockContainer& bc : generatedWorld) {
		std::ostringstream oss;
		oss << "./world/chunks/" << bc.globalpos.x << "x" << bc.globalpos.y << "y" << bc.globalpos.z << "z" << worldsize << ".txt";
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


void SingleGame::threadRenderJob() {
	while(true) {
		vector<ivec3> chunksInRender = chunksInRenderDistance(spectator.position);

		// Loop for derendering.
		for (int i = generatedWorld.size() - 1; i >= 0; i--) {
			bool continueRendering = false;
			for (int j = chunksInRender.size() - 1; j >= 0; j--) {
				if (chunksInRender[j] == generatedWorld[i].globalpos) {
					continueRendering = true;
					chunksInRender.erase(chunksInRender.begin() + j);
					break;
				}
			}

			if (!continueRendering) {
				renderer->derender(generatedWorld[i].root(), graphics->blockbuf);
				generatedWorld.erase(generatedWorld.begin() + i);
			}
		}

		// Loop for rendering and generating new chunks.
		for (ivec3& pos : chunksInRender) {
			BlockContainer bc = BlockContainer(pos, worldsize);
			loadOrGenerateTerrain(bc);

			
			renderer->render(bc.root(), graphics->blockbuf);

			generatedWorld.push_back(std::move(bc));
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void SingleGame::timestep() {
	static double last_time = getTime();
	double cur_time = getTime();
	double max_deltaTime = 1;
	double deltatime = cur_time - last_time;

	deltatime = std::min(max_deltaTime, deltatime);
	last_time = cur_time;
	
	spectator.timestep(cur_time, deltatime);
	graphics->viewbox->timestep(cur_time, deltatime);

	graphics->swap();
	
	playing = !controls->key_pressed('Q');

	
	// cout << spectator.position << ' ' << spectator.angle.x << ',' << spectator.angle.y << endl;
}


