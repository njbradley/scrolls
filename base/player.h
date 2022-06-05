#ifndef BASE_PLAYER
#define BASE_PLAYER

#include "common.h"
#include "plugins.h"

// plugin that handles button and mouse input, as this
// is normally done with the same library that creates
// the window
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

// simple player class, has no physics and
// flies around
class Spectator { public:
	vec3 position = vec3(-256,-256,-1280);
	vec2 angle = vec2(0,0);
	Controls* controller = nullptr;
	
	void timestep(float curtime, float deltatime);
};


#endif
