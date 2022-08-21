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
	GLuint uvbuffer;
	GLuint databuffer;
	vector<int*> owners;
	vector<GLfloat> poses;
	vector<GLfloat> uvs;
	vector<GLint> data;
	int first_empty;
	int empty_count = 0;
	bool changed = false;
	std::mutex lock;

	static const int POS_STRIDE = 18;
	static const int UV_STRIDE = 12;
	static const int DATA_STRIDE = 18;

	void set_buffers(GLuint posbuf, GLuint uvbuf, GLuint databuf);
	virtual void add(RenderFace arr[], int size, RenderIndex* outindex);
	virtual void del(RenderIndex* index);
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
	GLuint mvpMatID;
	GLuint blockTexID;
	GLuint suncolorID;
	GLuint sundirID;
	
	GLuint vertexarray;
	GLuint posbuffer;
	GLuint uvbuffer;
	GLuint databuffer;
	
	GLuint block_textures;
};




#endif
