#include "graphics.h"

#include <algorithm>

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

int GraphicsContext::get_texture_id(string path) {
	cout << "path " << path << endl;
	int i = 0;
	for (string curpath : block_texture_paths) {
		if (curpath == path or (curpath.length() > path.length() and curpath.substr(curpath.length()-path.length()) == path)) {
			cout << i << endl;
			return i;
		}
		i ++;
	}
	return -1;
}
