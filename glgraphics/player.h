#ifndef GLGLRAPHICS_PLAYER
#define GLGLRAPHICS_PLAYER

#include "common.h"
#include "base/player.h"

class GLControls : public Controls { public:
	PLUGIN(GLControls);
	
	GLControls();
	
	virtual bool key_pressed(int keycode);
	virtual bool mouse_button(int button);
	virtual ivec2 mouse_pos();
	virtual void mouse_pos(ivec2 newpos);
	virtual int scroll_rel();
};


#endif
