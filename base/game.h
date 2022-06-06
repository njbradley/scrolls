#ifndef BASE_GAME_H
#define BASE_GAME_H

#include "common.h"
#include "plugins.h"

#include "blocks.h"
#include "player.h"

class Chunk : public BlockContainer {
public:
	SingleGame* game;
	
	Chunk(SingleGame* newgame, ivec3 pos, int scale);
	
	virtual BlockContainer* find_neighbor(ivec3 pos, int goalscale);
};

// Game is the main class that runs the whole game
// setup_gameloop is called on the first frame, then timestep is called repeatedly
// until playing is false
class Game {
	BASE_PLUGIN(Game, ());
public:
	bool playing = true;
	
	virtual ~Game() {}
	
	virtual void setup_gameloop() = 0;
	virtual void timestep() = 0;
};

// Game implementation for single player mode
class SingleGame : public Game {
	PLUGIN(SingleGame);
public:
	SingleGame();
	virtual ~SingleGame();
	
	virtual void setup_gameloop();
	virtual void timestep();
protected:
	GraphicsContext* graphics;
	Renderer* renderer;
	
	// Allocate on the heap because we want to change render distance.
	vector<Chunk> generatedWorld;

	Spectator spectator;
	Controls* controls;
	
	friend class Chunk;
};


class SingleTreeGame : public Game {
	PLUGIN(SingleTreeGame);
public:
	SingleTreeGame();
	virtual ~SingleTreeGame();
	
	virtual void setup_gameloop();
	virtual void timestep();
protected:
	GraphicsContext* graphics;
	Renderer* renderer;
	TerrainGenerator* generator;
	
	BlockContainer world;
	
	Spectator spectator;
	Controls* controls;
};



#endif
