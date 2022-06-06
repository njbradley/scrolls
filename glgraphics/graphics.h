#ifndef GLGRAPHICS_GRAPHICS
#define GLGRAPHICS_GRAPHICS

#include "common.h"
#include "base/graphics.h"

#include <mutex>
#include <unordered_map>

class GLRenderBuf : public RenderBuf {
	PLUGIN(GLRenderBuf);
public:
	GLuint posbuffer;
	GLuint databuffer;
	vector<RenderPosData> posdata;
	vector<RenderFaceData> facedata;
	int first_empty = -1;
	bool changed = false;
	std::mutex lock;

	void set_buffers(GLuint verts, GLuint databuf);
	virtual int add(const RenderData& data);
	virtual void edit(int index, const RenderData& data);
	virtual void del(int index);
	virtual void sync();

};

class GLGraphics : public GraphicsContext {
	PLUGIN(GLGraphics);
public:
	GLGraphics();
	~GLGraphics();
	
	void init_graphics();
	void load_textures();
	void block_draw_call();
	virtual void swap();
protected:
	GLFWwindow* window;
	
	GLuint block_program;
	GLuint pMatID;
	GLuint mvMatID;
	GLuint blockTexID;
	GLuint suncolorID;
	GLuint sundirID;
	
	GLuint vertexarray;
	GLuint posbuffer;
	GLuint databuffer;
	
	GLuint block_textures;
};




#endif
