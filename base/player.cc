#include "player.h"

DEFINE_PLUGIN(Controls);

float mouseSpeed = 0.003f;

void Spectator::timestep(float curtime, float deltatime) {
	if (controller == nullptr) return;
	
	ivec2 screen_dims (100,100);
	
	angle += vec2(float(mouseSpeed) * vec2(screen_dims/2 - controller->mouse_pos()));
	
	//cout << horizontalAngle << ' ' << verticalAngle << endl;
	if (angle.y > 1.55f) {
		angle.y = 1.55f;
	}
	if (angle.y < -1.55) {
		angle.y = -1.55f;
	}
	if (angle.x > 6.28f) {
		angle.x = 0;
	}
	if (angle.x < 0) {
		angle.x = 6.28;
	}
	
	// Reset mouse position for next frame
	controller->mouse_pos(screen_dims/2);
	
	float nspeed = 1;
	
	glm::vec3 right = glm::vec3(
		sin(angle.x - 3.14f/2.0f),
		0,
		cos(angle.x - 3.14f/2.0f)
	);
	vec3 forward = -glm::vec3(
		-cos(angle.x - 3.14f/2.0f),
		0,
		sin(angle.x - 3.14f/2.0f)
	);
	
	// Up vector
	glm::vec3 up = glm::cross( right, forward );
	
	if (controller->key_pressed('W')){
		position += forward * deltatime * nspeed;
	}
	// Move backward
	if (controller->key_pressed('S')){
		position -= forward * deltatime * nspeed;
	}
	// Strafe right
	if (controller->key_pressed('D')){
		position += right * deltatime * nspeed;
	}
	// Strafe left
	if (controller->key_pressed('A')){
		position -= right * deltatime * nspeed;
	}
	if (controller->key_pressed(' ')){
		position += up * deltatime * nspeed;
	}
	if (controller->key_pressed(controller->KEY_SHIFT)){
		position -= up * deltatime * nspeed;
	}
}
