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
		uint16 texture = 0;
		uint16 other = 0;
		uint stuff = 0;
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

class GraphicsContext {
	BASE_PLUGIN(GraphicsContext, ());
public:
	ivec2 screen_dims = ivec2(1024, 728);
	
	GraphicsContext();
	~GraphicsContext();
	
	RenderBuf* blockbuf;
	
	void set_camera(vec3* newpos, vec2* newrot);
	
	virtual void swap() = 0;
protected:
	vec3* camera_pos;
	vec2* camera_rot;
};

#endif
