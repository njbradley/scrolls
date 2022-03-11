#include "graphics.h"

DEFINE_PLUGIN(RenderBuf);

DEFINE_PLUGIN(GraphicsContext);

GraphicsContext::GraphicsContext() {
	blockbuf = RenderBuf::plugnew(this);
	camera_pos = nullptr;
}

GraphicsContext::~GraphicsContext() {
	plugdelete(blockbuf);
}

void GraphicsContext::set_camera(vec3* newpos, vec2* newdir) {
	camera_pos = newpos;
	camera_dir = newdir;
}
