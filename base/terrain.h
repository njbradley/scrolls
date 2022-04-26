#ifndef BASE_TERRAIN
#define BASE_TERRAIN

#include "common.h"
#include "plugins.h"

#include <random>

using Blocktype = int;

const Blocktype BLOCK_NULL = -1;
const Blocktype BLOCK_SPLIT = -2;

using rand_gen = std::mt19937;
using int_dist = std::uniform_int_distribution<int>;
using float_dist = std::uniform_real_distribution<float>;
using double_dist = std::uniform_real_distribution<double>;


template <typename ... Seeds>
rand_gen init_generator(Seeds ... seeds) {
	std::tuple<Seeds...> tup (seeds...);
	std::seed_seq seq ((uint*)&tup, (uint*)&tup + sizeof(tup)/sizeof(int));
	rand_gen gen (seq);
	return gen;
}

int hash4(int seed, int a, int b, int c, int d);
int hash4(int seed, ivec3 pos, int d);
int hash4(int seed, ivec2 pos, int c, int d);

float randfloat(int seed, int a, int b, int c, int d);

ivec3 randivec3(int seed, int a, int b, int c, int d);
vec3 randvec3(int seed, int a, int b, int c, int d);
vec2 randvec2(int seed, int a, int b, int c, int d);

float perlin3d(vec3 pos, int seed, int layer);
float perlin2d(vec2 pos, int seed, int layer);

float fractal_perlin2d(vec2 pos, float scale, float divider, int seed, int layer);
float fractal_perlin3d(vec3 pos, float scale, float divider, int seed, int layer);

class TerrainGenerator {
public:
	int seed;
	vector<TerrainShape*> shapes;
	vector<TerrainDecorator*> decorators;
	int total_max = 0;
	int total_deriv = 0;
	int block_type = 3;
	
	TerrainGenerator(int seed);
	~TerrainGenerator();
	
	void generate_chunk(NodeView node);
	
	int gen_node(NodeView node);
	
	int get_height(ivec3 pos);
};

struct TerrainDecorator {
	BASE_PLUGIN(TerrainDecorator, (int seed));
public:
	int seed;
	
	TerrainDecorator(int nseed);
	virtual ~TerrainDecorator() {}
	
	virtual void decorate_chunk(NodeView node) = 0;
};

struct TerrainShape {
	BASE_PLUGIN(TerrainShape, (int seed));
public:
	int seed;
	float max_deriv;
	float max_val;
	
	TerrainShape(int nseed): seed(nseed) {}
	
	virtual float gen_value(vec3 pos) = 0;
};
	

// These are the methods that terrain shapes must implement
//
// struct TerrainShape {
//	returns a float value, below zero represents inside
// 	the shape, above zero represents outside
// 	static float gen_value(int seed, vec3 pos);
//
//	max_deriv is the max amount that the gen_value function can change over
// 	one unit in the x, y, or z directions. The idea is that the shape resolver
//	can limit the number of calls, because it can estimate how far away the surface
//	of the shape is and skip some calls to the gen_value func
// 	static float max_deriv();
//
//	The block type of the shape
// 	static Blocktype block_val();
// };

/*
template <typename ... Shapes>
struct ShapeResolver : public TerrainGenerator {
	PLUGIN(ShapeResolver);
	using TerrainGenerator::TerrainGenerator;
	
	float get_max_value(vec3 pos);
	float get_max_deriv();
	
	template <typename Shape>
	Blocktype gen_func(ivec3 pos, int scale);
	
	template <typename FirstShape, typename SecondShape, typename ... OtherShapes>
	Blocktype gen_block(NodeView node);
	template <typename Shape>
	Blocktype gen_block(NodeView node);
	template <typename ... CurShapes>
	Blocktype gen_block(NodeView node, Blocktype mytype);
	
	virtual void generate_chunk(NodeView node);
	virtual int get_height(ivec3 pos);
};*/




#endif
