#include "graphics.h"
#include <cstdio>
#include <filesystem>

#include "shader.h"
#include "textures.h"
#include "debug.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cstring>

EXPORT_PLUGIN(GLRenderBuf);

GLFWwindow* static_window;

void GLRenderBuf::set_buffers(GLuint verts, GLuint databuf) {
  posbuffer = verts;
  databuffer = databuf;
}

void check_emptylist(vector<RenderPosData>& posdata, int empty) {
  while (empty != -1) {
    int last = empty;
    empty = *((int*) &posdata[empty]);
    ASSERT(empty >= -1 and empty < (int)posdata.size() and last != empty);
  }
}

int GLRenderBuf::add(const RenderData& data) {
	std::lock_guard guard(lock);
  int index;
  if (first_empty != -1) {
    index = first_empty;
    first_empty = *((int*) &posdata[first_empty]);
    posdata[index] = data.posdata;
    facedata[index] = data.facedata;
  } else {
    index = posdata.size();
    posdata.push_back(data.posdata);
    facedata.push_back(data.facedata);
	}
  changed = true;
  return index;
}

void GLRenderBuf::edit(int index, const RenderData& data) {
	std::lock_guard guard(lock);
	posdata[index] = data.posdata;
  facedata[index] = data.facedata;
  changed = true;
}

void GLRenderBuf::del(int index) {
	std::lock_guard guard(lock);
  RenderData nulldata;
  posdata[index] = nulldata.posdata;
  facedata[index] = nulldata.facedata;
  *((int*) &posdata[index]) = first_empty;
  first_empty = index;
  changed = true;
}

void GLRenderBuf::sync() {
  std::lock_guard guard(lock);
  if (changed) {
    glBindBuffer(GL_ARRAY_BUFFER, posbuffer);
    glBufferData(GL_ARRAY_BUFFER, posdata.size()*sizeof(RenderPosData), &posdata[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, databuffer);
    glBufferData(GL_ARRAY_BUFFER, facedata.size()*sizeof(RenderFaceData), &facedata[0], GL_STATIC_DRAW);
    changed = false;
  }
}



EXPORT_PLUGIN(GLGraphics);

GLGraphics::GLGraphics() {
	init_graphics();
  load_textures();
}

GLGraphics::~GLGraphics() {
	glBindVertexArray(vertexarray);
  glDeleteBuffers(1, &posbuffer);
	glDeleteBuffers(1, &databuffer);
	
	glDeleteProgram(block_program);
	
	glDeleteVertexArrays(1, &vertexarray);
	
  glfwDestroyWindow(window);
  
	glfwTerminate();
  cout << "done"<< endl;
}

void GLAPIENTRY errorCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam ) {
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

void myGlfwErrorFunc(int error_code, const char* str) {
  std::cerr << "GLFW error (" << error_code << ") '" << str << "'" << endl;
}

bool update_window_size = false;
int new_width, new_height;
void window_size_callback(GLFWwindow* window, int width, int height) {
  new_width = width;
  new_height = height;
  update_window_size = true;
}

void GLGraphics::init_graphics() {
//   glfwSetErrorCallback(myGlfwErrorFunc);
  ASSERT_RUN(glfwInit());
  
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
  
  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  
  screen_dims.x = mode->width*6/8;
  screen_dims.y = mode->height*6/8;
  
	window = glfwCreateWindow(screen_dims.x, screen_dims.y, "Scrolls - An Adventure Game", nullptr, nullptr);
	static_window = window;
	ASSERT(window != nullptr);
	// glfwSetWindowPos(window, 100, 40);
	
  glfwMakeContextCurrent(window);
	
	ASSERT_RUN(glewInit() == GLEW_OK);
	
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(errorCallback, 0);
	
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
  // Hide the mouse and enable unlimited mouvement
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
  glfwSetWindowSizeCallback(window, window_size_callback);
  
	// Set the mouse at the center of the screen
  glfwPollEvents();
  glfwSetCursorPos(window, screen_dims.x/2, screen_dims.y/2);
	
	vec3 clearcolor = vec3(0.4f, 0.7f, 1.0f);
	glClearColor(0.4, 0.7, 1.0, 0.0f);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	glEnable(GL_CULL_FACE);
	// glDisable(GL_CULL_FACE);
	
	glGenVertexArrays(1, &vertexarray);
	
	block_program = LoadShadersGeo(
		pluginloader.find_path("shaders/block.vs").c_str(),
		pluginloader.find_path("shaders/block.fs").c_str(),
		pluginloader.find_path("shaders/block.gs").c_str()
	);
	
	pMatID = glGetUniformLocation(block_program, "Pmat");
  mvMatID = glGetUniformLocation(block_program, "MVmat");
	blockTexID = glGetUniformLocation(block_program, "textures");
	
  suncolorID = glGetUniformLocation(block_program, "suncolor");
  sundirID = glGetUniformLocation(block_program, "sundir");
  
	glBindVertexArray(vertexarray);
	GLuint blockbuffs[2];
	glGenBuffers(2, blockbuffs);
	posbuffer = blockbuffs[0];
	databuffer = blockbuffs[1];
	
	((GLRenderBuf*) blockbuf)->set_buffers(posbuffer, databuffer);
	
	glfwSwapInterval(1);
}

void GLGraphics::load_textures() {
  vector<string> paths;
  for (std::filesystem::path path : std::filesystem::directory_iterator(pluginloader.find_path("textures/blocks/"))) {
    paths.push_back(path.string());
  }
  
  block_textures = load_array(paths, 8);
}

void GLGraphics::block_draw_call() {
	if (camera_rot == nullptr or camera_pos == nullptr) return;
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glBindVertexArray(vertexarray);
	
	// Use our shader
	glUseProgram(block_program);
	
	vec3 direction(
		cos(camera_rot->y) * sin(camera_rot->x),
		sin(camera_rot->y),
		cos(camera_rot->y) * cos(camera_rot->x)
	);
	
	vec3 right = glm::vec3(
		sin(camera_rot->x - 3.14f/2.0f),
		0,
		cos(camera_rot->x - 3.14f/2.0f)
	);
	vec3 forward = -glm::vec3(
		-cos(camera_rot->x - 3.14f/2.0f),
		0,
		sin(camera_rot->x - 3.14f/2.0f)
	);
	
	vec3 up = glm::cross( right, forward );
	
	// Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(100.0f), (float) screen_dims.x/screen_dims.y, 0.1f, 100000.0f);
	glm::mat4 viewMatrix = glm::lookAt(*camera_pos, *camera_pos + direction, up);
	glm::mat4 modelMatrix = glm::mat4(1.0);
	glm::mat4 P = projectionMatrix;
	glm::mat4 MV = viewMatrix * modelMatrix;
	
	glUniformMatrix4fv(pMatID, 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(mvMatID, 1, GL_FALSE, &MV[0][0]);
	
  glUniform3f(suncolorID, viewbox->suncolor.x, viewbox->suncolor.y, viewbox->suncolor.z);
  vec3 sundir = MV * glm::vec4(viewbox->sundir.x, viewbox->sundir.y, viewbox->sundir.z, 0);
  glUniform3f(sundirID, sundir.x, sundir.y, sundir.z);
  
	//// Vertex attribures : position, rotation, and scale
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	// glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, posbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RenderPosData), (void*) offsetof(RenderPosData, pos));
	// glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(RenderPosData), (void*) offsetof(RenderPosPart, rot));
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(RenderPosData), (void*) offsetof(RenderPosData, scale));
	
	// Face data
	glBindBuffer(GL_ARRAY_BUFFER, databuffer);
	for (int i = 0; i < 6; i ++) {
		glEnableVertexAttribArray(2+i);
		glVertexAttribIPointer(2+i, 2, GL_UNSIGNED_INT, sizeof(RenderFaceData), (void*) (offsetof(RenderFaceData, faces) + i * sizeof(RenderFaceData)/6));
	}
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, block_textures);
	glUniform1i(blockTexID, 0);
	
	glDrawArrays(GL_POINTS, 0, ((GLRenderBuf*)blockbuf)->posdata.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	
	// debuglines->draw_call(P * MV);
}

void GLGraphics::swap() {
  
  if (update_window_size) {
    screen_dims.x = new_width;
    screen_dims.y = new_height;
    glViewport(0, 0, screen_dims.x, screen_dims.y);
  }
  // double start = getTime();
	blockbuf->sync();
  // cout << getTime() - start << " sync time " << endl;
  
	block_draw_call();

  ((GLDebugLines*)debuglines)->draw_call();

	glfwSwapBuffers(window);
	// ((GLRenderBuf*) blockbuf)->lock.unlock();

	glfwPollEvents();
}


	
	
bool getKey(char let) {
	return glfwGetKey(static_window, GLFW_KEY_Q) == GLFW_TRUE;
}
