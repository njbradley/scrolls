#ifndef SCROLLS_COMMON
#define SCROLLS_COMMON

#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>

using std::istream;
using std::ostream;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using glm::vec3;
using glm::ivec3;
using glm::vec2;

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint = unsigned int;

class Node;
class NodeView;
class NodeIter;
class Block;
class BlockView;
class BlockIter;
template <typename Iterator> class BlockIterable;
class BlockContainer;
class GraphicsContext;
class RenderBuf;
class Renderer;



#define ERR(MSG) std::cerr << "ERR: '" << MSG << "'" << endl \
	<< "at " << __PRETTY_FUNCTION__ << ": " << __LINE__ << " " __FILE__ << endl; \
	std::terminate();
#define ASSERT(X) if (!(X)) ERR("ASSERT(" #X ") failed");

struct Direction {
	static constexpr ivec3 dir_array[] = {
		{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
	static constexpr int all[] = {0,1,2,3,4,5};
	int index;
	
	Direction(int ind): index(ind) {}
	Direction(ivec3 npos): index(0) {
		for (const ivec3& pos : dir_array) {
			if (pos == npos) break;
			index ++;
		}
	}
	
	operator ivec3() { return dir_array[index]; }
	operator int() { return index; }
};

#endif
