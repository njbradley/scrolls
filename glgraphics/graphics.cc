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

void GLRenderBuf::set_buffers(GLuint posbuf, GLuint uvbuf, GLuint databuf) {
	posbuffer = posbuf;
	uvbuffer = uvbuf;
	databuffer = databuf;
}

void GLRenderBuf::add(RenderFace arr[], int size, RenderIndex* outindex) {
	std::lock_guard guard(lock);
	for (int i = 0; i < size; i ++) {
		int index;
		if (empty_count != 0) {
			index = first_empty;
			first_empty = data[first_empty*DATA_STRIDE];
			empty_count --;
		} else {
			index = owners.size();
			owners.push_back(nullptr);
			poses.resize(poses.size() + POS_STRIDE);
			uvs.resize(uvs.size() + UV_STRIDE);
			data.resize(data.size() + DATA_STRIDE);
		}
		
		arr[i].to_points(poses.begin() + (index*POS_STRIDE), uvs.begin() + (index*UV_STRIDE), data.begin() + (index*DATA_STRIDE));
		outindex->indices[i] = index;
		owners[index] = &outindex->indices[i];
	}
	
	outindex->size = size;
	outindex->buf = this;
	changed = true;
}

void GLRenderBuf::del(RenderIndex* index) {
	std::lock_guard guard(lock);
	for (int i = 0; i < index->size; i ++) {
		data[index->indices[i]*DATA_STRIDE] = first_empty;
		first_empty = index->indices[i];
		owners[index->indices[i]] = nullptr;
		empty_count ++;
	}
	index->size = 0;
	index->buf = nullptr;
	changed = true;
}

void GLRenderBuf::sync() {
	std::lock_guard guard(lock);

	if (changed) {
		if (empty_count > 0) {
			//cout << "cleaning empties, start size " << owners.size() << endl;
			int index = owners.size()-1;
			while (empty_count > 0 and index > 0) {
				if (owners[index] == nullptr) {
					index --;
					continue;
				}
				
				if (first_empty > index) {
					//cout << " skipping empty " << first_empty << endl;
					first_empty = data[first_empty*DATA_STRIDE];
					empty_count --;
					continue;
				}

				int new_index = first_empty;
				//cout << " copying " << index << " to empty " << first_empty << endl;
				first_empty = data[first_empty*DATA_STRIDE];
				empty_count --;
				
				owners[new_index] = owners[index];
				*owners[new_index] = new_index;
				std::copy(poses.begin() + index * POS_STRIDE, poses.begin() + (index+1) * POS_STRIDE, poses.begin() + new_index * POS_STRIDE);
				std::copy(uvs.begin() + index * UV_STRIDE, uvs.begin() + (index+1) * UV_STRIDE, uvs.begin() + new_index * UV_STRIDE);
				std::copy(data.begin() + index * DATA_STRIDE, data.begin() + (index+1) * DATA_STRIDE, data.begin() + new_index * DATA_STRIDE);

				index --;
			}
			int new_size = index+1;
			owners.resize(new_size);
			poses.resize(new_size*POS_STRIDE);
			uvs.resize(new_size*UV_STRIDE);
			data.resize(new_size*DATA_STRIDE);
			//cout << "cleaned empties, new size " << owners.size() << endl;
		}

		glBindBuffer(GL_ARRAY_BUFFER, posbuffer);
		glBufferData(GL_ARRAY_BUFFER, poses.size()*sizeof(GLfloat), &poses[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, uvs.size()*sizeof(GLfloat), &uvs[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, databuffer);
		glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(GLint), &data[0], GL_STATIC_DRAW);
		changed = false;

		//cout << "final size: " << owners.size() << endl;
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

	block_program = LoadShaders(
			pluginloader()->find_path("shaders/block.vs").c_str(),
			pluginloader()->find_path("shaders/block.fs").c_str()
			//pluginloader()->find_path("shaders/block.gs").c_str()
			);

	mvpMatID = glGetUniformLocation(block_program, "MVP");
	blockTexID = glGetUniformLocation(block_program, "textures");
	suncolorID = glGetUniformLocation(block_program, "suncolor");
	sundirID = glGetUniformLocation(block_program, "sundir");

	glBindVertexArray(vertexarray);
	GLuint blockbuffs[3];
	glGenBuffers(3, blockbuffs);
	posbuffer = blockbuffs[0];
	uvbuffer = blockbuffs[1];
	databuffer = blockbuffs[2];

	((GLRenderBuf*) blockbuf)->set_buffers(posbuffer, uvbuffer, databuffer);

	glfwSwapInterval(1);
}

void GLGraphics::load_textures() {
	for (std::filesystem::path path : std::filesystem::directory_iterator(pluginloader()->find_path("textures/blocks/"))) {
		block_texture_paths.push_back(path.string());
	}

	block_textures = load_array(block_texture_paths, 8);
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
	
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(100.0f), (float) screen_dims.x/screen_dims.y, 0.1f, 100000.0f);
	glm::mat4 viewMatrix = glm::lookAt(*camera_pos, *camera_pos + direction, up);
	glm::mat4 modelMatrix = glm::mat4(1.0);
	glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
	
	glUniformMatrix4fv(mvpMatID, 1, GL_FALSE, &MVP[0][0]);

	glUniform3f(suncolorID, viewbox->suncolor.x, viewbox->suncolor.y, viewbox->suncolor.z);
	vec3 sundir = viewbox->sundir;
	glUniform3f(sundirID, sundir.x, sundir.y, sundir.z);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, posbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, (void*) 0);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*2, (void*) 0);
	glBindBuffer(GL_ARRAY_BUFFER, databuffer);
	glVertexAttribIPointer(2, 3, GL_INT, sizeof(GLint)*3, (void*) 0);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, block_textures);
	glUniform1i(blockTexID, 0);
	
	glDrawArrays(GL_TRIANGLES, 0, ((GLRenderBuf*)blockbuf)->poses.size() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
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
	return glfwGetKey(static_window, let) == GLFW_TRUE;
}
