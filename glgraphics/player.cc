#include "player.h"

EXPORT_PLUGIN(GLControls);

static int scroll_rel_val = 0;

extern GLFWwindow* static_window;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	scroll_rel_val += yoffset;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	
}

GLControls::GLControls() {
	glfwSetScrollCallback(static_window, scroll_callback);
	// glfwSetKeyCallback(window, key_callback);
	
	KEY_SHIFT = GLFW_KEY_LEFT_SHIFT;
	KEY_CTRL = GLFW_KEY_LEFT_CONTROL;
	KEY_ALT = GLFW_KEY_LEFT_ALT;
	KEY_TAB = GLFW_KEY_TAB;
}


bool GLControls::key_pressed(int keycode) {
	return glfwGetKey(static_window, keycode) == GLFW_PRESS;
}

bool GLControls::mouse_button(int button) {
	return glfwGetMouseButton(static_window, button) == GLFW_PRESS;
}

ivec2 GLControls::mouse_pos() {
	double x, y;
	glfwGetCursorPos(static_window, &x, &y);
	return ivec2(x,y);
}

void GLControls::mouse_pos(ivec2 newpos) {
	glfwSetCursorPos(static_window, newpos.x, newpos.y);
}

int GLControls::scroll_rel() {
	int val = scroll_rel_val;
	scroll_rel_val = 0;
	return val;
}
