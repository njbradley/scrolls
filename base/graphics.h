#ifndef SCROLLS_GRAPHICS
#define SCROLLS_GRAPHICS

#include "common.h"
#include "plugins.h"


struct RenderFace {
	vec3 center;
	vec3 xaxis;
	vec3 yaxis;
	int sunlight;
	int blocklight;
	int texture;

	RenderFace() {}
	RenderFace(vec3 center, vec3 xaxis, vec3 yaxis, int sunlight, int blocklight, int texture);

	template <typename floatIter, typename intIter>
	void to_points(floatIter poses, floatIter uvs, intIter data);
};



struct RenderIndex {
	RenderBuf* buf = nullptr;
	int size = 0;
	int indices[6];

	void clear();
	bool isvalid() const;
};

class RenderBuf {
	BASE_PLUGIN(RenderBuf, ());
public:
	
	virtual void add(RenderFace arr[], int size, RenderIndex* outindex) = 0;
	virtual void del(RenderIndex* index) = 0;
	virtual void sync() = 0;
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
	// returnes the texture id of the path (or -1 if it doesn't exist)
	int get_texture_id(string path);
	
	virtual void swap() = 0;
protected:
	vec3* camera_pos;
	vec2* camera_rot;
	vector<string> block_texture_paths;
};





// INLINE FUNCTIONS


template <typename floatIter, typename intIter>
void RenderFace::to_points(floatIter poses, floatIter uvs, intIter data) {
	for (int i = 0; i < 6; i ++) {
		int index = (i < 3 ? i : i-1) % 4;
		int x = index/2;
		int y = (index+1)%4/2;
		vec3 pos = center + float(x*2-1) * xaxis + float(y*2-1) * yaxis;
		*(poses++) = pos.x;
		*(poses++) = pos.y;
		*(poses++) = pos.z;
		*(uvs++) = x;
		*(uvs++) = y;
		*(data++) = sunlight;
		*(data++) = blocklight;
		*(data++) = texture;
	}
}







#endif
