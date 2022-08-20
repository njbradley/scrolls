#ifndef SCROLLS_COMMON
#define SCROLLS_COMMON

#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

using std::istream;
using std::ostream;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using glm::vec3;
using glm::ivec3;
using glm::vec2;
using glm::ivec2;
using glm::quat;

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
class FreeNode;
class NodePtr;
class NodeView;
class FreeNodeView;
template <typename NodePtrT> class NodeIter;
template <typename NodePtrT> class ChildIter;
class Block;
class BlockData;
class BlockView;
template <typename NodePtrT> class BlockIter;
template <typename Iterator> class BlockIterable;
class BlockContainer;
class GraphicsContext;
class RenderBuf;
class Renderer;
class TerrainGenerator;
class TerrainDecorator;
class TerrainShape;
class IHitCube;
class Game;
class Chunk;
class SingleGame;
class Pool;
class BlockFileSystem;
class BlockFileFormat;


#define ERR(MSG) std::cerr << "ERR: '" << MSG << "'" << endl \
	<< "at " << __PRETTY_FUNCTION__ << ": " << __LINE__ << " " __FILE__ << endl; \
	std::terminate();

#ifdef SCROLLS_DEBUG
#define ASSERT(X) if (!(X)) {ERR("ASSERT(" #X ") failed");}
#define ASSERT_RUN(X) ASSERT(X)
#else
#define ASSERT(X) ;
#define ASSERT_RUN(X) (X);
#endif


struct Direction {
	static constexpr ivec3 dir_array[] = {
		{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
	static constexpr int all[] = {0,1,2,3,4,5};
	int index;
	
	constexpr Direction(int ind): index(ind) {}
	constexpr Direction(ivec3 npos): index(0) {
		for (const ivec3& pos : dir_array) {
			if (pos == npos) break;
			index ++;
		}
	}
	
	constexpr operator ivec3() const { return dir_array[index]; }
	constexpr operator int() const { return index; }
	constexpr Direction operator-() const { return Direction((index+3)%6); }
};

bool getKey(char let);
double getTime();

inline ostream& operator<<(ostream& out, ivec3 pos) {
	out << pos.x << ',' << pos.y << ',' << pos.z;
	return out;
}

inline ostream& operator<<(ostream& out, vec3 pos) {
	out << pos.x << ',' << pos.y << ',' << pos.z;
	return out;
}

inline int safefloor(float val) {
	return int(val) - (val < 0 and val != int(val));
}

inline ivec3 safefloor(vec3 pos) {
	return ivec3(safefloor(pos.x), safefloor(pos.y), safefloor(pos.z));
}

inline ivec2 safefloor(vec2 pos) {
	return ivec2(safefloor(pos.x), safefloor(pos.y));
}

#endif
