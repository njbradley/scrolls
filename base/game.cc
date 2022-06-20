#include "game.h"
#include "graphics.h"
#include "rendering.h"
#include "blocks.h"
#include "blockiter.h"
#include "terrain.h"
#include "threadpool/pool.h"
#include "debug.h"
#include "entity.h"
#include "fileformat.h"

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


EXPORT_PLUGIN(SingleTreeGame);
EXPORT_PLUGIN(SingleGame);

int PARAM(worldsize) = 256;

const int renderdistance = worldsize;  // FIX THIS VARIABLE NAME
const int chunkloadingstart = 3;

const int chunks = (2*chunkloadingstart + 1) * (2*chunkloadingstart + 1) *(2*chunkloadingstart + 1);

const bool overwrite_saves = false;

const bool multi_thread_loading_chunks = true;


Pool* jobPool;


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
  filesystem = BlockFileSystem::plugnew("world/chunks/");
  jobPool = new Pool(4);

	generatedWorld.reserve(chunks);

 	generator = TerrainGenerator::plugnew(12345);

}

SingleGame::~SingleGame() {
  run_thread = false;
  nick.join();
  delete jobPool;
  plugdelete(filesystem);
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
		if (!filesystem->from_file(bc)) {
			generator->generate_chunk(bc);
      filesystem->to_file(bc);
		}
}


void SingleGame::setup_gameloop() {
	
	cout << "starting test " << BDIMS << endl;
	
	double start = getTime();
  	cout << generator->get_height(ivec3(0,0,0)) << " get_height" << endl;
  
	// for (BlockContainer& bc : generatedWorld) {
	// 	jobPool.pushJob( [&] {
	// 		loadOrGenerateTerrain(bc);
	// 		renderer->render(bc, graphics->blockbuf);
	// 	});
	// }
  
	// cout << getTime() - start << " Time terrain " << endl;
	// start = getTime();
  
  
	cout << getTime() - start << " Time render " << endl;
	
	start = getTime();
	int num = 0;

	for (BlockContainer& bc : generatedWorld ) {
		for (BlockView view : BlockIterable<BlockIter<NodeView>>(bc)) {
			num ++;
		}
	}
	cout << getTime() - start << " Time iter (num blocks): " << num << endl;
	
  start = getTime();
  num = 0;
  for (BlockContainer& bc : generatedWorld ) {
		for (NodePtr node : BlockIterable<BlockIter<NodePtr>>(bc)) {
			num ++;
		}
	}
  cout << getTime() - start << " Time iter (num blocks): " << num << endl;
  
	spectator.controller = controls;
	graphics->set_camera(&spectator.position, &spectator.angle);
	nick = std::thread(&SingleGame::threadRenderJob, this);

}


void SingleGame::threadRenderJob() {
	while(run_thread) {
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
					renderer->derender(generatedWorld[i], graphics->blockbuf);
					generatedWorld.erase(generatedWorld.begin() + i);
				}
			}

			// Loop for rendering and generating new chunks.
			for (const ivec3& pos : chunksInRender) {
				if (multi_thread_loading_chunks) {
						jobPool->pushJob([pos, this] {
							{
								if (SingleGame::chunkStillValid(pos)) {
									Chunk bc(this, pos, worldsize);
									loadOrGenerateTerrain(bc);

									renderer->render(bc, graphics->blockbuf);
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
						renderer->render(bc, graphics->blockbuf);
						generatedWorld.push_back(std::move(bc));
					}
				}
			}
		}
    if (!run_thread) return;
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
  
	spectator.timestep(cur_time, deltatime);
	graphics->viewbox->timestep(cur_time, deltatime);

  double start = getTime();
	graphics->swap();
  double swap_time = getTime() - start;
	// std::cout << getTime() - t1 << std::endl;

  debugstr << "Times: swap:" << swap_time << endl;
  
  debuglines->clear();
  debuglines->draw(vec2(-0.99, 0.95), debugstr.str());
  
  playing = !controls->key_pressed('Q');
}





const int loading_resolution = worldsize/4;


SingleTreeGame::SingleTreeGame(): world(ivec3(-worldsize/2, -worldsize/2, -worldsize/2), worldsize) {
	graphics = GraphicsContext::plugnew();
	renderer = Renderer::plugnew();
	controls = Controls::plugnew();
  generator = TerrainGenerator::plugnew(12345);
  threadpool = new Pool(4);
}

SingleTreeGame::~SingleTreeGame() {
  delete threadpool;
  plugdelete(generator);
  plugdelete(controls);
  plugdelete(renderer);
  plugdelete(graphics);
}


void SingleTreeGame::setup_gameloop() {
	
	cout << "starting test " << BDIMS << endl;
	
	double start = getTime();
  cout << generator->get_height(ivec3(0,0,0)) << " get_height" << endl;
  
  generator->generate_chunk(world);
  
	cout << getTime() - start << " Time terrain " << endl;
	start = getTime();
  
	renderer->render(world, graphics->blockbuf);
  
	cout << getTime() - start << " Time render " << endl;
	
	start = getTime();
	int num = 0;
  
  for (BlockView view : BlockIterable<BlockIter<NodeView>>(world)) {
		num ++;
	}
	
  cout << getTime() - start << " Time iter (num blocks): " << num << endl;
	
  start = getTime();
  num = 0;
	for (NodePtr node : BlockIterable<BlockIter<NodePtr>>(world)) {
		num ++;
	}
  cout << getTime() - start << " Time iter (num blocks): " << num << endl;
  
	spectator.controller = controls;
	graphics->set_camera(&spectator.position, &spectator.angle);
}

void SingleTreeGame::generate_new_world(NodeView newnode, NodeView oldroot, bool generate, bool copy) {
  // cout << newnode.position << ' ' << oldroot.position << ' ' <<(newnode.position - oldroot.position) % newnode.scale  << endl;
  if ((newnode.position - oldroot.position) % newnode.scale != ivec3(0,0,0)) {
    if (!newnode.haschildren()) {
      newnode.split();
    }
    for (int i = 0; i < BDIMS3; i ++) {
      generate_new_world(newnode.child(i), oldroot, generate, copy);
    }
  } else if (oldroot.contains(newnode)) {
    if (copy) {
      NodeView src = oldroot.get_global(newnode.position, newnode.scale);
      if (src.scale > newnode.scale) {
        // cout << "copying " << newnode.position << ' ' << newnode.scale << " from " << src.position << ' ' << src.scale << endl;
        newnode.copy_tree(src);
      } else {
        // cout << "swapping " << newnode.position << ' ' << newnode.scale << endl;
        newnode.swap_tree(src);
      }
      for (Direction dir : Direction::all) {
        IHitCube sidebox = IHitCube(newnode) + ivec3(dir);
        if (!sidebox.collides(oldroot)) {
          NodeView sidenode = newnode.get_global(sidebox.position, sidebox.scale);
          if (sidenode.isvalid()) {
            for (NodePtr node : NodePtr(sidenode).iter<DirBlockIter>(-dir)) {
              node.on_change();
            }
            for (NodePtr node : NodePtr(newnode).iter<DirBlockIter>(dir)) {
              node.on_change();
            }
          }
        }
      }
    }
  } else {
    if (generate) {
      // cout << "generating " << newnode.position << ' ' << newnode.scale << endl;
      generator->generate_chunk(newnode);
    }
  }
}

void SingleTreeGame::check_loading() {
  if (generation_lock.try_lock()) {
    std::unique_lock guard(generation_lock, std::adopt_lock);
    IHitCube goalbox (world.midpoint() - loading_resolution*3/4, loading_resolution*3/2);
    if (!goalbox.contains(ivec3(spectator.position))) {
      ivec3 localpos = ivec3(spectator.position) - goalbox.midpoint();
      ivec3 rolldir = glm::sign(localpos * 2 / loading_resolution);
      ivec3 newpos = world.position + rolldir * loading_resolution;
      // relocate_world(newpos);
      threadpool->pushJob([this, newpos] () {
        relocate_world(newpos);
      });
    }
  }
}

void SingleTreeGame::relocate_world(ivec3 newpos) {
  std::lock_guard guard(generation_lock);
  
  cout << "Changing from " << world.position << " to " << newpos << endl;
  BlockContainer newworld (newpos, world.scale);
  
  cout << "generating new world" << endl;
  generate_new_world(newworld, world, true, false);
  renderer->render(newworld, graphics->blockbuf);
  
  {
    std::lock_guard guard(world_lock);
    cout << "copying old world over" << endl;
    generate_new_world(newworld, world, false, true);
    newworld.swap(world);
  }
  renderer->render(world, graphics->blockbuf);
  
  cout << "derendering " << endl;
  renderer->derender(newworld, graphics->blockbuf);
}

void SingleTreeGame::timestep() {
	static double last_time = getTime();
	double cur_time = getTime();
	double deltatime = cur_time - last_time;
	last_time = cur_time;
	check_loading();
  std::stringstream debugstr;
  debugstr << "FPS: " << 1 / deltatime << endl;
  debugstr << "spectatorpos: " << spectator.position << endl;
  debuglines->clear();
  debuglines->draw(vec2(-0.99, 0.95), debugstr.str());
  
	spectator.timestep(cur_time, deltatime);
	graphics->viewbox->timestep(cur_time, deltatime);
	graphics->swap();
	playing = !controls->key_pressed('Q');
}
