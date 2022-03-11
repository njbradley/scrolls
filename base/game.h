#ifndef BASE_GAME_H
#define BASE_GAME_H

#include "common.h"
#include "plugins.h"

#include "blocks.h"

class Game {
	BASE_PLUGIN(Game, ());
public:
	bool playing = true;
	
	virtual ~Game() {}
	
	virtual void setup_gameloop() = 0;
	virtual void timestep() = 0;
};


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
	
	BlockContainer world;
};

#endif
