#include "graphics.h"

DEFINE_PLUGIN(RenderBuf);

DEFINE_PLUGIN(GraphicsContext);

RenderData::RenderData() {
	posdata.scale = -1;
}



DEFINE_AND_EXPORT_PLUGIN(ViewBox);

void ViewBox::timestep(float curtime, float deltatime) {
	sundir = vec3(0,-1,0);
	suncolor = vec3(1,1,1);
}





GraphicsContext::GraphicsContext() {
	blockbuf = RenderBuf::plugnew();
	viewbox = ViewBox::plugnew();
	camera_pos = nullptr;
}

GraphicsContext::~GraphicsContext() {
	plugdelete(blockbuf);
	plugdelete(viewbox);
}

void GraphicsContext::set_camera(vec3* newpos, vec2* newrot) {
	camera_pos = newpos;
	camera_rot = newrot;
}
