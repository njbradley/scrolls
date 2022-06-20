#include "blocks.h"
#include "game.h"
#include "debug.h"


int main(int numargs, char** args) {
	pluginloader()->load(numargs-1, args+1);
	
	// Game* game = new SingleGame();
	Game* game = Game::plugnew();
	debuglines = DebugLines::plugnew();
	
	game->setup_gameloop();
	while (game->playing) {
		game->timestep();
	}
	
	plugdelete(debuglines);
	plugdelete(game);
	// delete game;
	
	return 0;
}
