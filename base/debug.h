#ifndef BASE_DEBUG
#define BASE_DEBUG

#include "common.h"
#include "plugins.h"

class DebugLines {
	BASE_PLUGIN(DebugLines, ());
	
	virtual ~DebugLines() {}
	
	virtual void draw(vec2 start, vec2 end) = 0;
	virtual void clear() = 0;
	
	void draw(vec2 pos, char let);
	void draw(vec2 pos, const string& text);
};

extern DebugLines* debuglines;

#endif
