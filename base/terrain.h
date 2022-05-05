#ifndef BASE_TERRAIN
#define BASE_TERRAIN

#include "common.h"
#include "plugins.h"

#include <random>


/*

The terrain generation has three steps:

Terrain layer generation: First, terrain layers are generated. These
	are 3d heatmaps that represent a shape, ie the level of the land

Base terrain generation: Next, these layers are used to create the
	base terrain. For example, the level of the land would be used
	to create a layer of grass, a layer of dirt, and stone below that.

Terrain decoration: Finally, the terrain is decorated, with objects
	and other addons, like trees and structures.

*/




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

vec3 randvec3(int seed, int a, int b, int c, int d);
vec2 randvec2(int seed, int a, int b, int c, int d);

float perlin3d(vec3 pos, int seed, int layer);
float perlin2d(vec2 pos, int seed, int layer);

float fractal_perlin2d(vec2 pos, float scale, float divider, int seed, int layer);
float fractal_perlin3d(vec3 pos, float scale, float divider, int seed, int layer);

class TerrainGenerator {
	BASE_PLUGIN(TerrainGenerator, (int seed));
public:
	int seed;
	
	TerrainGenerator(int seed);
	virtual ~TerrainGenerator() {}
	
	virtual void generate_chunk(NodeView node) = 0;
	virtual int get_height(ivec3 pos) = 0;
};

class TerrainDecorator {
	BASE_PLUGIN(TerrainDecorator, (int seed));
public:
	int seed;
	
	TerrainDecorator(int nseed);
	virtual ~TerrainDecorator() {}
	
	virtual void decorate_chunk(NodeView node) = 0;
};

// These are the methods that terrain shapes must implement
//
// struct TerrainShape {
//	returns a float value, below zero represents inside
// 	the shape, above zero represents outside
// 	static float gen_value(ShapeContext* ctx, vec3 pos);
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

struct ShapeContext {
	int seed;
	float layers[10];
	
	template <typename ... Shapes>
	void set_layers(vec3 pos);
};

template <typename ... Shapes>
struct ShapeGroup {
	
};

// template <typename ... Layers, typename ... Shapes>
// struct ShapeResolver<ShapeGroup<Layers...>,ShapeGroup<Shapes...>> : public TerrainGenerator {
// 	PLUGIN(ShapeResolver<ShapeGroup<Layers...>,ShapeGroup<Shapes...>>);

template <typename ... Shapes>
struct ShapeResolver : public TerrainGenerator {
	PLUGIN(ShapeResolver<Shapes...>);
	using TerrainGenerator::TerrainGenerator;
	
	float get_max_value(vec3 pos);
	float get_max_deriv();
	
	ShapeContext get_context(vec3 pos);
	
	template <typename Shape>
	Blocktype gen_func(ShapeContext* ctx, ivec3 pos, int scale);
	
	template <typename FirstShape, typename SecondShape, typename ... OtherShapes>
	Blocktype gen_block(NodeView node, ShapeContext* ctx);
	template <typename Shape>
	Blocktype gen_block(NodeView node, ShapeContext* ctx);
	template <typename ... OtherShapes>
	Blocktype gen_block(NodeView node);
	template <typename ... CurShapes>
	Blocktype gen_block(NodeView node, Blocktype mytype);
	
	virtual void generate_chunk(NodeView node);
	virtual int get_height(ivec3 pos);
};




#endif
