#include "graphics.h"

DEFINE_PLUGIN(RenderBuf);

DEFINE_PLUGIN(GraphicsContext);

RenderData::RenderData() {
	posdata.scale = -1;
}

GraphicsContext::GraphicsContext() {
	blockbuf = RenderBuf::plugnew();
	camera_pos = nullptr;
}

GraphicsContext::~GraphicsContext() {
	plugdelete(blockbuf);
}

void GraphicsContext::set_camera(vec3* newpos, vec2* newrot) {
	camera_pos = newpos;
	camera_rot = newrot;
}
