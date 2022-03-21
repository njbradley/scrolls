#ifndef GLGRAPHICS_GRAPHICS
#define GLGRAPHICS_GRAPHICS

#include "common.h"
#include "base/graphics.h"


class GLRenderBuf : public RenderBuf {
	PLUGIN(GLRenderBuf);
public:
	GLuint posbuffer;
	GLuint databuffer;
	int num_points = 0;
	int allocated_space = 0;
	vector<int> empties;
	
	void set_buffers(GLuint verts, GLuint databuf, int start_size);
	virtual int add(const RenderData& data);
	virtual void edit(int index, const RenderData& data);
	virtual void del(int index);
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
	
	GLuint vertexarray;
	GLuint posbuffer;
	GLuint databuffer;
	
	GLuint block_textures;
};




#endif
