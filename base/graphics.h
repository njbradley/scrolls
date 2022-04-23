#ifndef SCROLLS_GRAPHICS
#define SCROLLS_GRAPHICS

#include "common.h"
#include "plugins.h"

#pragma pack(push, 0)

struct RenderPosData {
	vec3 pos;
	float scale;
};

struct RenderFaceData {
	struct {
		uint16 texture;
		uint16 stuff;
		uint8 sunlight;
		uint8 blocklight;
		uint16 other;
	} faces[6];
};

struct RenderData {
	union {
		RenderPosData posdata;
		float posarr[sizeof(RenderPosData)/sizeof(float)];
	};
	union {
		RenderFaceData facedata;
		uint facearr[sizeof(RenderFaceData)/sizeof(uint)];
	};
	RenderData();
};

#pragma pack(pop)

class RenderBuf {
	BASE_PLUGIN(RenderBuf, ());
public:
	
	virtual int add(const RenderData& data) = 0;
	virtual void edit(int index, const RenderData& data) = 0;
	virtual void del(int index) = 0;
};

class ViewBox {
	BASE_PLUGIN(ViewBox, ());
public:
	vec3 suncolor;
	vec3 sundir;
	
	virtual void timestep(float curtime, float deltatime);
};

class GraphicsContext {
	BASE_PLUGIN(GraphicsContext, ());
public:
	ivec2 screen_dims = ivec2(1024, 728);
	
	GraphicsContext();
	virtual ~GraphicsContext();
	
	RenderBuf* blockbuf;
	ViewBox* viewbox;
	
	void set_camera(vec3* newpos, vec2* newrot);
	
	virtual void swap() = 0;
protected:
	vec3* camera_pos;
	vec2* camera_rot;
};

#endif
