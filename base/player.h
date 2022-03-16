#ifndef BASE_PLAYER
#define BASE_PLAYER

#include "common.h"
#include "plugins.h"

class Controls { public:
	BASE_PLUGIN(Controls, ());
	
	int KEY_SHIFT;
	int KEY_CTRL;
	int KEY_ALT;
	int KEY_TAB;
	
	virtual bool key_pressed(int keycode) = 0;
	virtual bool mouse_button(int button) = 0;
	virtual ivec2 mouse_pos() = 0;
	virtual void mouse_pos(ivec2 newpos) = 0;
	virtual int scroll_rel() = 0;
};

class Spectator { public:
	vec3 position;
	vec2 angle;
	Controls* controller = nullptr;
	
	void timestep(float curtime, float deltatime);
};


#endif
