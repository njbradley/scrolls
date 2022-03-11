#include "game.h"
#include "graphics.h"
#include "rendering.h"

DEFINE_PLUGIN(Game);


EXPORT_PLUGIN(SingleGame);

SingleGame::SingleGame(): world(ivec3(0,0,0), 64) {
	graphics = GraphicsContext::plugnew();
	renderer = Renderer::plugnew();
}

SingleGame::~SingleGame() {
	plugdelete(graphics);
	plugdelete(renderer);
}

void SingleGame::setup_gameloop() {
	
}

void SingleGame::timestep() {
	
	
}
