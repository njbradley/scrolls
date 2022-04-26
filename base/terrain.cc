#include "terrain.h"
#include "blocks.h"

#include <sstream>
#include <algorithm>

int hash4(int seed, int a, int b, int c, int d) {
	int sum = seed + 13680553*a + 47563643*b + 84148333*c + 80618477*d;
	sum = (sum ^ (sum >> 13)) * 750490907;
  return sum ^ (sum >> 16);
}

int hash4(int seed, ivec3 pos, int d) {
	return hash4(seed, pos.x, pos.y, pos.z, d);
}

int hash4(int seed, ivec2 pos, int c, int d) {
	return hash4(seed, pos.x, pos.y, c, d);
}

float randfloat(int seed, int a, int b, int c, int d) {
  return (float)(hash4(seed,a,b,c,d)%1000000) / 1000000;
}

ivec3 randivec3(int seed, int a, int b, int c, int d) {
  return ivec3(hash4(seed, a, b, c, d*3), hash4(seed, a, b, c, d*3+1), hash4(seed, a, b, c, d*3+2));
}

vec3 randvec3(int seed, int a, int b, int c, int d) {
  return vec3(randfloat(seed, a, b, c, d*3), randfloat(seed, a, b, c, d*3+1), randfloat(seed, a, b, c, d*3+2));
}

vec2 randvec2(int seed, int a, int b, int c, int d) {
  return vec2(randfloat(seed, a, b, c, d*2), randfloat(seed, a, b, c, d*2+1));
}

double lerp(double a0, double a1, double w) {
    return (1.0f - w)*a0 + w*a1;
}

float interpolate(float a0, float a1, float w) {
  //return (a1 - a0) * w + a0;
  // Use this cubic interpolation [[Smoothstep]] instead, for a smooth appearance:
  return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
  
  // Use [[Smootherstep]] for an even smoother result with a second derivative equal to zero on boundaries:
  //return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
}



float gridval3d(ivec3 ipos, vec3 pos, int seed, int layer) {
  vec3 pvec = glm::normalize(randvec3(seed, ipos.x, ipos.y, ipos.z, layer));
  return glm::dot(pos - vec3(ipos), pvec);
}

float gridval2d(ivec2 ipos, vec2 pos, int seed, int layer) {
  vec2 pvec = glm::normalize(randvec2(seed, ipos.x, ipos.y, 0, layer));
  return glm::dot(pos - vec2(ipos), pvec);
}


float perlin3d(vec3 pos, int seed, int layer) {
  ivec3 ipos = safefloor(pos);
  vec3 localpos = pos - vec3(ipos);
  
  float val11 = interpolate(gridval3d(ipos+ivec3(0,0,0), pos, seed, layer), gridval3d(ipos+ivec3(1,0,0), pos, seed, layer), localpos.x);
  float val12 = interpolate(gridval3d(ipos+ivec3(0,1,0), pos, seed, layer), gridval3d(ipos+ivec3(1,1,0), pos, seed, layer), localpos.x);
  float val1 = interpolate(val11, val12, localpos.y);
  
  float val21 = interpolate(gridval3d(ipos+ivec3(0,0,1), pos, seed, layer), gridval3d(ipos+ivec3(1,0,1), pos, seed, layer), localpos.x);
  float val22 = interpolate(gridval3d(ipos+ivec3(0,1,1), pos, seed, layer), gridval3d(ipos+ivec3(1,1,1), pos, seed, layer), localpos.x);
  float val2 = interpolate(val21, val22, localpos.y);
  
  return interpolate(val1, val2, localpos.z);
}
  
float perlin2d(vec2 pos, int seed, int layer) {
  ivec2 ipos = safefloor(pos);
  vec2 localpos = pos - vec2(ipos);
  
  float val1 = interpolate(gridval2d(ipos, pos, seed, layer), gridval2d(ipos+ivec2(1,0), pos, seed, layer), localpos.x);
  float val2 = interpolate(gridval2d(ipos+ivec2(0,1), pos, seed, layer), gridval2d(ipos+ivec2(1,1), pos, seed, layer), localpos.x);
  return interpolate(val1, val2, localpos.y);
}

float fractal_perlin2d(vec2 pos, float scale, float divider, int seed, int layer) {
  float val = 0;
  int i = 0;
  while (scale > 1) {
    val += perlin2d(pos / scale, seed, layer*100 + i) * scale;
    scale /= divider;
    i ++;
  }
  return val;
}

float fractal_perlin3d(vec3 pos, float scale, float divider, int seed, int layer) {
  float val = 0;
  int i = 0;
  while (scale > 1) {
    val += perlin3d(pos / scale, seed, layer*100 + i) * scale;
    scale /= divider;
    i ++;
  }
  return val;
}


bool shapesorter(TerrainShape* shape1, TerrainShape* shape2) {
	return shape1->max_val > shape2->max_val;
}

TerrainGenerator::TerrainGenerator(int newseed): seed(newseed) {
	allplugnew(shapes, seed);
	
	std::sort(shapes.begin(), shapes.end(), shapesorter);
	for (TerrainShape* shape : shapes) {
		total_max += shape->max_val;
		total_deriv += shape->max_deriv;
	}
	
	allplugnew(decorators, seed);
	
	// // testing shapes
	// for (TerrainShape* shape : shapes) {
	// 	float calc_max = 0;
	// 	float calc_deriv = 0;
	//
	// 	for (int i = 0; i < 10000000; i ++) {
	// 		vec3 pos = randivec3(seed, i, 0, 0, 0)%1000000;
	// 		int change_dist = hash4(seed, i, i, i, 0) % 64;
	// 		float val1 = shape->gen_value(pos);
	// 		float val2 = shape->gen_value(pos + glm::normalize(randvec3(seed, i, i, 0, 0)) * float(change_dist));
	// 		calc_max = std::max(calc_max, std::max(std::abs(val1), std::abs(val2)));
	// 		calc_deriv = std::max(calc_deriv, std::abs(val1 - val2) / change_dist);
	// 	}
	//
	// 	cout << "Checked shape " << shape->get_plugindef()->id << " max: " << calc_max << " / " << shape->max_val
	// 	<< " deriv: " << calc_deriv << " / " << shape->max_deriv << endl;
	// }
}

TerrainGenerator::~TerrainGenerator() {
	for (TerrainShape* shape : shapes) {
		plugdelete(shape);
	}
	for (TerrainDecorator* decor : decorators) {
		plugdelete(decor);
	}
}

enum TerrainValue {
	TERRAIN_AIR = 0,
	TERRAIN_FILL = 1,
	TERRAIN_SPLIT = -1
};

void TerrainGenerator::generate_chunk(NodeView node) {
	double start = getTime();
	gen_node(node);
	double mid = getTime();
	for (TerrainDecorator* decor : decorators) {
		decor->decorate_chunk(node);
	}
	cout << "Decorate time: " << getTime() - mid << "  gen time: " << mid - start << endl;
}

int TerrainGenerator::gen_node(NodeView node) {
	float val_scale = float(node.scale)/2;
	vec3 val_pos = vec3(node.globalpos) + val_scale;
	float cur_val = val_pos.y + 16;
	float max_val = total_max + val_scale;
	float max_deriv = total_deriv + 1;
	
	for (TerrainShape* shape : shapes) {
		if (std::abs(cur_val) - max_val > max_deriv * val_scale * 1.5f) {
			node.set_block(new Block((cur_val < 0) * block_type));
			return cur_val < 0;
		}
		if (std::abs(cur_val) + max_val < max_deriv * val_scale * 1.5f) {
			break;
		}
		cur_val += shape->gen_value(val_pos);
		max_val -= shape->max_val;
	}
	
	if (val_scale < 1 or std::abs(cur_val) > max_deriv * val_scale * 1.5f) {
		node.set_block(new Block((cur_val < 0) * block_type));
		return cur_val < 0;
	}
	
	node.split();
	int total_result = gen_node(node.child(0));
	for (int i = 1; i < BDIMS3; i ++) {
		int result = gen_node(node.child(i));
		total_result = (result != total_result) ? TERRAIN_SPLIT : result;
	}
	if (total_result != TERRAIN_SPLIT) {
		node.join();
		node.set_block(new Block(total_result * block_type));
		return total_result;
	}
	return TERRAIN_SPLIT;
}



int TerrainGenerator::get_height(ivec3 pos) {
	return 0;
}




DEFINE_PLUGIN(TerrainDecorator);

TerrainDecorator::TerrainDecorator(int newseed): seed(newseed) {
	
}



DEFINE_PLUGIN(TerrainShape);












template <int scale, int height, int layer>
struct Perlin2d : TerrainShape {
	PLUGIN(Perlin2d);
	Perlin2d(int seed): TerrainShape(seed) {
		max_deriv = 1.45f * height / scale;
		max_val = height / 2.0f;
	}
	
	virtual float gen_value(vec3 pos) {
		return perlin2d(vec2(pos.x, pos.z) / float(scale), seed, layer) * height * 1.45f;
	}
};

template <int scale, int height, int layer>
struct Perlin3d : TerrainShape {
	PLUGIN(Perlin3d);
	Perlin3d(int seed): TerrainShape(seed) {
		max_deriv = 1.15f * height / scale;
		max_val = height / 2.0f;
	}
	
	virtual float gen_value(vec3 pos) {
		 return perlin3d(pos / float(scale) + randvec3(seed, layer, 143, 211, 566), seed, layer) * height * 1.15f;
	}
};

// template <int scale, int layer>
// struct RidgePerlin3d : Perlin3d<scale, layer> {
// 	PLUGIN(RidgePerlin3d);
// 	using Perlin3d<scale, layer>::Perlin3d;
// 	virtual float gen_value(int seed, vec3 pos) {
// 		return scale - std::abs(Perlin3d<scale, layer>::gen_value(seed, pos));
// 	}
// };

using Layer1 = Perlin2d<512,256,0>;
EXPORT_PLUGIN_TEMPLATE(Perlin2d<512,256,0>);

using Layer2 = Perlin3d<32,64,1>;
EXPORT_PLUGIN_TEMPLATE(Perlin3d<32,64,1>);

using Layer3 = Perlin3d<9,18,2>;
EXPORT_PLUGIN_TEMPLATE(Perlin3d<9,18,2>);

// HeightFalloff<Add<Perlin3d<64,0>, Perlin3d<9,1>, RidgePerlin3d<32,4>, Perlin2d<512,9>>, 1>;








struct PlainsDecorator : TerrainDecorator {
	PLUGIN(PlainsDecorator);
	using TerrainDecorator::TerrainDecorator;
	
	void add_grass(NodeView bottom, NodeView top) {
		if ((bottom.hasblock() and bottom.block()->value == 0) or (top.hasblock() and top.block()->value != 0)) {
			return;
		}
		if (bottom.continues() and top.continues()) {
			for (int x = 0; x < BDIMS; x ++) {
				for (int z = 0; z < BDIMS; z ++) {
					add_grass(bottom.child(ivec3(x, 0, z)), bottom.child(ivec3(x, 1, z)));
					add_grass(bottom.child(ivec3(x, 1, z)), top.child(ivec3(x, 0, z)));
					add_grass(top.child(ivec3(x, 0, z)), top.child(ivec3(x, 1, z)));
				}
			}
		} else if (bottom.continues()) {
			for (int x = 0; x < BDIMS; x ++) {
				for (int z = 0; z < BDIMS; z ++) {
					add_grass(bottom.child(ivec3(x, 0, z)), bottom.child(ivec3(x, 1, z)));
					add_grass(bottom.child(ivec3(x, 1, z)), top);
				}
			}
		} else if (top.continues()) {
			for (int x = 0; x < BDIMS; x ++) {
				for (int z = 0; z < BDIMS; z ++) {
					add_grass(top.child(ivec3(x, 0, z)), top.child(ivec3(x, 1, z)));
					add_grass(bottom, top.child(ivec3(x, 0, z)));
				}
			}
		} else {
			bottom.block()->value = 2;
			int start_height = bottom.globalpos.y;
			while (start_height - bottom.globalpos.y < 3
					and bottom.moveto(bottom.globalpos - ivec3(0,1,0), 1)
					and bottom.block()->value != 0
					and bottom.scale < 8) {
				if (bottom.block()->value != 2) {
					bottom.block()->value = 1;
				}
			}
		}
	}
	
	virtual void decorate_chunk(NodeView node) {
		if (node.continues()) {
			for (int x = 0; x < BDIMS; x ++) {
				for (int z = 0; z < BDIMS; z ++) {
					add_grass(node.child(ivec3(x, 0, z)), node.child(ivec3(x, 1, z)));
				}
			}
		}
	}
};

EXPORT_PLUGIN(PlainsDecorator);


/*
template <typename ... Shapes>
float ShapeResolver<Shapes...>::get_max_value(vec3 pos) {
	float vals[sizeof...(Shapes)] = {Shapes::gen_value(seed, pos)...};
	return *std::max_element(vals, vals+sizeof...(Shapes));
}

template <typename ... Shapes>
float ShapeResolver<Shapes...>::get_max_deriv() {
	float vals[sizeof...(Shapes)] = {Shapes::max_deriv()...};
	return *std::max_element(vals, vals+sizeof...(Shapes));
}

template <typename ... Shapes>
template <typename Shape>
Blocktype ShapeResolver<Shapes...>::gen_func(ivec3 globalpos, int scale) {
	float val = Shape::gen_value(seed, vec3(globalpos) + float(scale)/2);
	float level_needed = float(scale-1)/2 * Shape::max_deriv() * 2.1f;
	if (val >= level_needed) {
		return Shape::block_val();
	} else if (val <= -level_needed) {
		return BLOCK_NULL;
	} else {
		return BLOCK_SPLIT;
	}
}


template <typename ... Shapes>
template <typename FirstShape, typename SecondShape, typename ... OtherShapes>
Blocktype ShapeResolver<Shapes...>::gen_block(NodeView node) {
	Blocktype val = gen_func<FirstShape>(node.globalpos, node.scale);
	if (val == BLOCK_NULL) {
		return gen_block<SecondShape,OtherShapes...>(node);
	} else {
		return gen_block<FirstShape,SecondShape,OtherShapes...>(node, val);
	}
}

template <typename ... Shapes>
template <typename Shape>
Blocktype ShapeResolver<Shapes...>::gen_block(NodeView node) {
	Blocktype val = gen_func<Shape>(node.globalpos, node.scale);
	if (val == BLOCK_NULL) {
		val = 0;
	}
	return gen_block<Shape>(node, val);
}



template <typename ... Shapes>
template <typename ... CurShapes>
Blocktype ShapeResolver<Shapes...>::gen_block(NodeView node, Blocktype myval) {
  if (myval != BLOCK_SPLIT or node.scale == 1) {
		if (myval == BLOCK_NULL) myval = 0;
		node.set_block(new Block(myval));
    return myval;
  } else {
		node.split();
    Blocktype val = gen_block<CurShapes...>(node.child(0));
    bool all_same = val != -1;
		for (int i = 1; i < BDIMS3; i ++) {
			Blocktype newval = gen_block<CurShapes...>(node.child(i));
			all_same = all_same and newval == val;
		}
    if (all_same) {
			node.join();
			node.set_block(new Block(val));
      return val;
    } else {
      return -1;
    }
  }
}



template <typename ... Shapes>
void ShapeResolver<Shapes...>::generate_chunk(NodeView node) {
	// std::stringstream ss;
	gen_block<Shapes...>(node);
	// node.from_file(ss);
}

template <typename ... Shapes>
int ShapeResolver<Shapes...>::get_height(ivec3 pos) {
	float val;
	while (std::abs(val = get_max_value(vec3(pos) + 0.5f)) > 2) {
		if (val > 0) {
			pos.y += std::max(1.0f, val / get_max_deriv() / 2);
		} else {
			pos.y += std::min(-1.0f, val / get_max_deriv() / 2);
		}
	}
	return pos.y;
}










template <typename Shape, int value>
struct SolidType : public Shape {
	static Blocktype block_val() {
		return value;
	}
};

template <typename Shape, int x, int y, int z>
struct Shift : public Shape {
	static float gen_value(int seed, vec3 pos) {
		return Shape::gen_value(seed, pos - vec3(x,y,z));
	}
};

template <typename Shape, int x, int y, int z>
struct Scale : public Shape {
	static float gen_value(int seed, vec3 pos) {
		return Shape::gen_value(seed, pos * vec3(x,y,z));
	}
	
	static float max_deriv() {
		return Shape::max_deriv() * std::max(x, std::max(y, z));
	}
};

template <typename ... Shapes>
struct Add {
	static float gen_value(int seed, vec3 pos) {
		return (Shapes::gen_value(seed, pos) + ...);
	}
	
	static float max_deriv() {
		return (Shapes::max_deriv() + ...);
	}
};

template <typename Shape1, int num, int denom = 1>
struct AddVal : public Shape1 {
	static constexpr float val = float(num) / denom;
	static float gen_value(int seed, vec3 pos) {
		return Shape1::gen_value(seed, pos) + val;
	}
};

template <typename Shape1, int num, int denom = 1>
struct MulVal : public Shape1 {
	static constexpr float val = float(num) / denom;
	static float gen_value(int seed, vec3 pos) {
		return Shape1::gen_value(seed, pos) * val;
	}
	
	static float max_deriv() {
		return Shape1::max_deriv() * std::abs(val);
	}
};






template <typename Shape, int num_falloff, int denom_falloff = 1>
struct HeightFalloff : public Shape {
	static float gen_value(int seed, vec3 pos) {
		return Shape::gen_value(seed, pos) - pos.y * num_falloff / denom_falloff;
	}
	
	static float max_deriv() {
		return Shape::max_deriv() + num_falloff / denom_falloff;
	}
};

template <int scale, int layer>
struct Perlin2d {
	static float gen_value(int seed, vec3 pos) {
		return perlin2d(vec2(pos.x, pos.z) / float(scale), seed, layer) * scale;
	}
	
	static float max_deriv() {
		return 1.5f;
	}
};

template <int max_scale, int divider, int layer>
struct FractalPerlin2d {
	static float gen_value(int seed, vec3 pos) {
		float val = 0;
	  int i = 0;
		int scale = max_scale;
	  while (scale > 1) {
	    val += perlin2d(vec2(pos.x, pos.z) / float(scale), seed, layer*100 + i) * scale;
	    scale /= divider;
	    i ++;
	  }
		val -= pos.y * max_deriv();
	  return val;
	}
	
	static float max_deriv() {
		return std::max(1.0, std::log(max_scale) / std::log(divider));
	}
};

template <int scale, int layer>
struct Perlin3d {
	static float gen_value(int seed, vec3 pos) {
		return perlin3d(pos / float(scale) + randvec3(seed, layer, 143, 211, 566), seed, layer) * scale * 2;
	}
	
	static float max_deriv() {
		return 1.5f;
	}
};

template <int max_scale, int divider, int layer>
struct FractalPerlin3d {
	static float gen_value(int seed, vec3 pos) {
		// return perlin3d(pos / float(max_scale), seed, layer) * max_scale;
		float val = 0;
	  int i = 0;
		int scale = max_scale;
	  while (scale > 1) {
	    val += perlin3d(pos / float(scale), seed, layer*100 + i) * scale;
	    scale /= divider;
	    i ++;
	  }
	  return val;
	}
	
	static float max_deriv() {
		return std::log(max_scale) / std::log(divider);
	}
};


template <int scale, int layer>
struct RidgePerlin3d : Perlin3d<scale, layer> {
	static float gen_value(int seed, vec3 pos) {
		return scale - std::abs(Perlin3d<scale, layer>::gen_value(seed, pos));
	}
};








template <int height>
struct FlatTerrain {
	static float gen_value(int seed, vec3 pos) {
		return height - pos.y;
	}
	
	static float max_deriv() {
		return 1;
	}
};

template <int height, int slope_numer, int slope_denom = 1>
struct SlopedTerrain {
	static float gen_value(int seed, vec3 pos) {
		return height + pos.x * slope_numer / slope_denom - pos.y;
	}
	
	static float max_deriv() {
		return std::max(std::abs((float)slope_numer/slope_denom), 1.0f);
	}
};








using FlatWorld = ShapeResolver<
	SolidType<FlatTerrain<32>, 1>
>;
// EXPORT_PLUGIN_TEMPLATE(FlatWorld);

using LandLevel = HeightFalloff<Add<Perlin3d<64,0>, Perlin3d<9,1>, RidgePerlin3d<32,4>, Perlin2d<512,9>>, 1>;

using TestWorld = ShapeResolver<
	SolidType<LandLevel, 3>,
	SolidType<Shift<AddVal<LandLevel, -3>, 0,2,0>, 1>,
	SolidType<Shift<AddVal<LandLevel, -5>, 0,5,0>, 2>
>;
EXPORT_PLUGIN_TEMPLATE(TestWorld);

*/

class NullDecorator : public TerrainDecorator {
	PLUGIN(NullDecorator);
public:
	using TerrainDecorator::TerrainDecorator;
	virtual void decorate_chunk(NodeView node) {}
};

// EXPORT_PLUGIN(NullDecorator);
