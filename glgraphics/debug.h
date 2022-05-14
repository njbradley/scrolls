#ifndef GL_DEBUG
#define GL_DEBUG

#include "common.h"
#include "base/debug.h"

class GLDebugLines : public DebugLines {
	PLUGIN(GLDebugLines);
public:
	vector<GLfloat> data;
	
	GLuint vertexid;
	GLuint program;
	GLuint buffer;
	GLuint mvp_uniform;
	
	GLDebugLines();
	~GLDebugLines();
	
	virtual void draw(vec2 start, vec2 end);
	virtual void clear();
	
	void draw_call();
};

#endif
