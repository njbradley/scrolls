#include "debug.h"
#include "shader.h"

GLDebugLines::GLDebugLines() {
	glGenVertexArrays(1, &vertexid);
	
	program = LoadShaders(
		pluginloader.find_path("shaders/lines.vs").c_str(),
		pluginloader.find_path("shaders/lines.fs").c_str()
	);
	
	glBindVertexArray(vertexid);
	glGenBuffers(1, &buffer);
}

GLDebugLines::~GLDebugLines() {
	glBindVertexArray(vertexid);
	glDeleteBuffers(1, &buffer);
	
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vertexid);
}

void GLDebugLines::draw(vec2 start, vec2 end) {
	data.push_back(start.x);
	data.push_back(start.y);
	data.push_back(end.x);
	data.push_back(end.y);
}

void GLDebugLines::clear() {
	data.clear();
}

void GLDebugLines::draw_call() {
	glBindVertexArray(vertexid);
	glUseProgram(program);
	
	// glUniformMatrix4fv(mvp_uniform, 1, GL_FALSE, &MVP[0][0]);
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), &data[0], GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*2, (void*)0);
	// glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*6, (void*) (sizeof(GLfloat)*3));
	
	glDrawArrays(GL_LINES, 0, data.size()/2);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

EXPORT_PLUGIN(GLDebugLines);
	
