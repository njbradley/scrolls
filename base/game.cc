#include "game.h"
#include "graphics.h"
#include "rendering.h"
#include "blocks.h"
#include "blockiter.h"
#include "terrain.h"
#include "threadpool/pool.h"
#include "debug.h"
#include "entity.h"

#include <set>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <thread> 
#include <mutex>
#include <condition_variable>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>

DEFINE_PLUGIN(Game);


EXPORT_PLUGIN(SingleGame);

const int worldsize = 64;

const int renderdistance = worldsize;  // FIX THIS VARIABLE NAME
const int chunkloadingstart = 3;

const int chunks = (2*chunkloadingstart + 1) * (2*chunkloadingstart + 1) *(2*chunkloadingstart + 1);

const bool overwrite_saves = false;

const bool multi_thread_loading_chunks = true;


Pool jobPool(16);


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

 	generator = TerrainGenerator::plugnew(12345);

}

SingleGame::~SingleGame() {
	plugdelete(graphics);
	plugdelete(renderer);
	plugdelete(controls);
}

bool SingleGame::chunkStillValid(vec3 chunkPos) {
	int x_rem = static_cast<int>(spectator.position.x) % renderdistance;
	int y_rem = static_cast<int>(spectator.position.y) % renderdistance;
	int z_rem = static_cast<int>(spectator.position.z) % renderdistance;

	if (x_rem < 0) {
		x_rem += renderdistance;
	}

	if (y_rem < 0) {
		y_rem += renderdistance;
	}

	if(z_rem < 0) {
		z_rem += renderdistance;
	}

	ivec3 player_chunk_pos = ivec3(spectator.position.x - x_rem, spectator.position.y - y_rem, spectator.position.z - z_rem);

	return (chunkPos.x <= player_chunk_pos.x + (chunkloadingstart * worldsize) && chunkPos.x >= player_chunk_pos.x - (chunkloadingstart * worldsize))
		&&  (chunkPos.y <= player_chunk_pos.y + (chunkloadingstart * worldsize) && chunkPos.y >= player_chunk_pos.y - (chunkloadingstart * worldsize))
		&&  (chunkPos.z <= player_chunk_pos.z + (chunkloadingstart * worldsize) && chunkPos.z >= player_chunk_pos.z - (chunkloadingstart * worldsize));
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
		// generatedWorld.push_back(Chunk(this, ivec3(x, y, z)*worldsize, worldsize));

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
		oss << "./world/chunks/" << bc.position.x << "x" << bc.position.y << "y" << bc.position.z << "z" << worldsize << ".txt";

		struct stat buf;
		// If the file does not exist, create terrain.
		// Otherwise, read from file.
		if (stat(oss.str().c_str(), &buf) != 0) {
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
  	cout << generator->get_height(ivec3(0,0,0)) << " get_height" << endl;
  
	// for (BlockContainer& bc : generatedWorld) {
	// 	jobPool.pushJob( [&] {
	// 		loadOrGenerateTerrain(bc);
	// 		renderer->render(bc.root(), graphics->blockbuf);
	// 	});
	// }
  
	// cout << getTime() - start << " Time terrain " << endl;
	// start = getTime();
  
  
	cout << getTime() - start << " Time render " << endl;
	
	start = getTime();
	int num = 0;

	for (BlockContainer& bc : generatedWorld ) {
		for (BlockView view : BlockIterable<BlockIter<NodeView>>(bc.root())) {
			num ++;
		}
	}
	cout << getTime() - start << " Time iter (num blocks): " << num << endl;
	
  start = getTime();
  num = 0;
  for (BlockContainer& bc : generatedWorld ) {
		for (NodePtr node : BlockIterable<BlockIter<NodePtr>>(bc.root())) {
			num ++;
		}
	}
  cout << getTime() - start << " Time iter (num blocks): " << num << endl;
  
	spectator.controller = controls;
	graphics->set_camera(&spectator.position, &spectator.angle);
	nick = std::thread(&SingleGame::threadRenderJob, this);

}


void SingleGame::threadRenderJob() {
	while(true) {
		vector<ivec3> chunksInRender;
		{
			std::lock_guard lck(isChunkLoading_lock);
			chunksInRender = chunksInRenderDistance(spectator.position);

			// Loop for derendering.
			cout << generatedWorld.size() << endl;
			for (int i = generatedWorld.size() - 1; i >= 0; i--) {
				bool continueRendering = false;
				for (int j = chunksInRender.size() - 1; j >= 0; j--) {
					if (chunksInRender[j] == generatedWorld[i].position) {
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
			for (const ivec3& pos : chunksInRender) {
				if (multi_thread_loading_chunks) {
						jobPool.pushJob([pos, this] { 
							{
								if (SingleGame::chunkStillValid(pos)) {
									Chunk bc(this, pos, worldsize);
									loadOrGenerateTerrain(bc);

									renderer->render(bc.root(), graphics->blockbuf);
									std::lock_guard lck(isChunkLoading_lock);
									generatedWorld.push_back(std::move(bc));
								}
							}
						});
				} else {
					cout << pos << endl;
					if (SingleGame::chunkStillValid(pos)) {
						Chunk bc(this, pos, worldsize);
						loadOrGenerateTerrain(bc);
						renderer->render(bc.root(), graphics->blockbuf);
						generatedWorld.push_back(std::move(bc));
					}
				}
			}
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
	// std::cout << getTime() - t1 << std::endl;

	playing = !controls->key_pressed('Q');

	
	// cout << spectator.position << ' ' << spectator.angle.x << ',' << spectator.angle.y << endl;
}






const int loading_resolution = worldsize/2;

EXPORT_PLUGIN(SingleTreeGame);


SingleTreeGame::SingleTreeGame(): world(ivec3(-worldsize, -worldsize, -worldsize), worldsize*2) {
	graphics = GraphicsContext::plugnew();
	renderer = Renderer::plugnew();
	controls = Controls::plugnew();
  generator = TerrainGenerator::plugnew(12345);
}

SingleTreeGame::~SingleTreeGame() {
  plugdelete(generator);
  plugdelete(controls);
  plugdelete(renderer);
  plugdelete(graphics);
}


void SingleTreeGame::setup_gameloop() {
	
	cout << "starting test " << BDIMS << endl;
	
	double start = getTime();
  cout << generator->get_height(ivec3(0,0,0)) << " get_height" << endl;
  
  generator->generate_chunk(world.root());
  
	cout << getTime() - start << " Time terrain " << endl;
	start = getTime();
  
	renderer->render(world.root(), graphics->blockbuf);
  
	cout << getTime() - start << " Time render " << endl;
	
	start = getTime();
	int num = 0;
  
  for (BlockView view : BlockIterable<BlockIter<NodeView>>(world.root())) {
		num ++;
	}
	
  cout << getTime() - start << " Time iter (num blocks): " << num << endl;
	
  start = getTime();
  num = 0;
	for (NodePtr node : BlockIterable<BlockIter<NodePtr>>(world.root())) {
		num ++;
	}
  cout << getTime() - start << " Time iter (num blocks): " << num << endl;
  
	spectator.controller = controls;
	graphics->set_camera(&spectator.position, &spectator.angle);
}

// void check_loading() {
//   IHitCube goalbox (world.root().midpoint() - loading_resolution/2, loading_resolution/2)
//   if (!goalbox.contains(spectator.position)) {
//     ivec3 rolldir = glm::sign(

void SingleTreeGame::timestep() {
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
}
