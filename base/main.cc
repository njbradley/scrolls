#include "arrblocks.h"
#include "game.h"

int main() {
	pluginloader.load();
	Game* game = Game::plugnew();
	
	game->setup_gameloop();
	while (game->playing) {
		game->timestep();
	}
	
	plugdelete(game);
	
	return 0;
}
