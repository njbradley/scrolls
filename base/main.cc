#include "blocks.h"
#include "game.h"
#include "debug.h"


int main() {
	pluginloader.load();
	
	Game* game = Game::plugnew();
	debuglines = DebugLines::plugnew();
	
	game->setup_gameloop();
	while (game->playing) {
		game->timestep();
	}
	
	plugdelete(game);
	plugdelete(debuglines);
	
	return 0;
}
