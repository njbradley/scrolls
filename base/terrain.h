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





BlockData* const BLOCK_NULL = (BlockData*)(-1);
BlockData* const BLOCK_SPLIT = (BlockData*)(-2);

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

float perlin3d(int seed, float x, float y, float z);
float perlin2d(int seed, float x, float y);



struct TerrainContext;
struct TerrainValue;
struct Layers;
struct ShapeValue;
struct BiomeResult;

using LayerFunc = void (*) (TerrainContext* ctx, Layers* outlayers, vec3 pos);
using ShapeFunc = ShapeValue (*) (TerrainContext* ctx, const Layers* layers, vec3 pos);
using BiomeFunc = BiomeResult (*) (TerrainContext* ctx, const Layers* layers, ivec3 pos, int scale);


struct TerrainValue {
	float value = 0;
	float deriv = 0;
	
	TerrainValue();
	TerrainValue(float value, float deriv);
	
	TerrainValue operator+(const TerrainValue& other) const;
	TerrainValue operator-(const TerrainValue& other) const;
	TerrainValue operator*(const TerrainValue& other) const;
	TerrainValue operator/(const TerrainValue& other) const;
	
	TerrainValue operator+(float val) const;
	TerrainValue operator-(float val) const;
	TerrainValue operator*(float val) const;
	TerrainValue operator/(float val) const;
	
	TerrainValue& operator+=(const TerrainValue& other);
	TerrainValue& operator-=(const TerrainValue& other);
	TerrainValue& operator*=(const TerrainValue& other);
	TerrainValue& operator/=(const TerrainValue& other);
	
	TerrainValue& operator+=(float val);
	TerrainValue& operator-=(float val);
	TerrainValue& operator*=(float val);
	TerrainValue& operator/=(float val);

	bool operator<(const TerrainValue& other) const;
	bool operator>(const TerrainValue& other) const;
	
	static TerrainValue lerp(const TerrainValue& val1, const TerrainValue& val2, float amount);
	static TerrainValue min(const TerrainValue& val1, const TerrainValue& val2);
	static TerrainValue max(const TerrainValue& val1, const TerrainValue& val2);
	static TerrainValue abs(const TerrainValue& val);
};


struct ShapeValue : protected TerrainValue {
	using TerrainValue::value;
	using TerrainValue::deriv;
	LayerFunc layergen;
	BlockData* btype;
	BiomeFunc biome;
	TerrainValue falloff;
	
	ShapeValue() {}
	ShapeValue(const TerrainValue& terrvalue, BlockData* btype, LayerFunc layergen = nullptr, BiomeFunc biome = nullptr, TerrainValue falloff = TerrainValue(1,0));
	
	bool operator<(const ShapeValue& other) const;
	bool operator>(const ShapeValue& other) const;
	
	BlockData* blocktype(int scale);
	bool needs_split(int scale);
};


struct TerrainContext {
	int seed;
};

struct Layers {
	static constexpr int num_layers = 4;
	TerrainValue ground_level;
	TerrainValue stone_level;
	TerrainValue wetness;
	TerrainValue temperature;

	TerrainValue* layers() {
		return (TerrainValue*)this;
	}
};

struct BiomeResult {
	LayerFunc nextlayergen;
	BiomeFunc nextbiome;
	BlockData* blocktype;
	bool needs_split;
};



struct LerpLayerGen {
	vec3 position;
	float scale;
	struct FloatLayers {
		float values[Layers::num_layers];
	} samples[8];
	float max_deriv[Layers::num_layers];
	
	LerpLayerGen(TerrainContext* ctx, LayerFunc shape, vec3 samplepos, float scale);
	void add_value(Layers* layers, vec3 pos);
};







class TerrainGenerator {
	BASE_PLUGIN(TerrainGenerator, (int seed));
public:
	int seed;
	
	TerrainGenerator(int seed);
	virtual ~TerrainGenerator() {}
	
	virtual void generate_chunk(NodeView node, int depth) = 0;
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










struct BiomeGenerator : public TerrainGenerator {
	PLUGIN(BiomeGenerator);
	using TerrainGenerator::TerrainGenerator;
	static BiomeFunc root_biome;
	static LayerFunc root_layergen;
	
	virtual void generate_chunk(NodeView node, int depth);
	virtual int get_height(ivec3 pos);
	BlockData* gen_node(NodeView node, LerpLayerGen* prevlayergen, LayerFunc layergen, BiomeFunc biome, int depth);
};
































/*
struct TerrainContext {
	int seed;
	vec3 sample_pos;
	
	TerrainContext(int nseed, vec3 curpos);
};

using LayerFunc = TerrainValue (*) (TerrainContext* ctx, vec3 pos);


struct ShapeContext : TerrainContext {
	static const int num_layers = 10;
	
	ShapeContext(int nseed, vec3 curpos, LayerFunc* funcarr, int num_funcs);
	
	TerrainValue layer(int index);
private:
	LayerFunc layerfuncs[num_layers];
	TerrainValue layers[num_layers];
};

using ShapeFunc = ShapeValue (*) (ShapeContext* ctx, vec3 pos);


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
// 	static BlockData* block_val();
// };

template <LayerFunc ... Layers>
struct LayerResolver {
	LayerFunc layers[sizeof...(Layers)] = {Layers...};
	int size = sizeof...(Layers);
};

template <typename Layers, ShapeFunc ... Shapes>
struct ShapeResolver : public TerrainGenerator {
	PLUGIN(ShapeResolver<Layers,Shapes...>);
	using TerrainGenerator::TerrainGenerator;
	Layers layers;
	
	virtual void generate_chunk(NodeView node, int depth);
	BlockData* gen_node(NodeView node, ShapeFunc* shapes, int num_shapes, int max_depth);
	
	virtual int get_height(ivec3 pos);
};

*/


// template <typename ... Shapes>
// struct ShapeResolver : public TerrainGenerator {
// 	PLUGIN(ShapeResolver<Shapes...>);
// 	using TerrainGenerator::TerrainGenerator;
//
// 	float get_max_value(vec3 pos);
// 	float get_max_deriv();
//
// 	ShapeContext get_context(vec3 pos);
//
// 	template <typename Shape>
// 	BlockData* gen_func(ShapeContext* ctx, ivec3 pos, int scale);
//
// 	template <typename FirstShape, typename SecondShape, typename ... OtherShapes>
// 	BlockData* gen_block(NodeView node, ShapeContext* ctx);
// 	template <typename Shape>
// 	BlockData* gen_block(NodeView node, ShapeContext* ctx);
// 	template <typename ... OtherShapes>
// 	BlockData* gen_block(NodeView node);
// 	template <typename ... CurShapes>
// 	BlockData* gen_block(NodeView node, BlockData* mytype);
//
// 	virtual void generate_chunk(NodeView node);
// 	virtual int get_height(ivec3 pos);
// };







#endif
