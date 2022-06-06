#include "blocks.h"
#include "game.h"
#include "debug.h"


int main() {
	pluginloader.load();
	
	Game* game = new SingleGame();
	debuglines = DebugLines::plugnew();
	
	game->setup_gameloop();
	while (game->playing) {
		game->timestep();
	}
	
	plugdelete(debuglines);
	delete game;
	
	return 0;
}
