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


DEFINE_PLUGIN(TerrainGenerator);

TerrainGenerator::TerrainGenerator(int newseed): seed(newseed) {
	
}



DEFINE_PLUGIN(TerrainDecorator);

TerrainDecorator::TerrainDecorator(int newseed): seed(newseed) {
	
}










template <typename ... Shapes>
ShapeContext ShapeResolver<Shapes...>::get_context(vec3 pos) {
	ShapeContext context {seed};
	return context;
}

template <typename ... Shapes>
float ShapeResolver<Shapes...>::get_max_value(vec3 pos) {
	ShapeContext context = get_context(pos);
	float vals[sizeof...(Shapes)] = {Shapes::gen_value(&context, pos)...};
	return *std::max_element(vals, vals+sizeof...(Shapes));
}

template <typename ... Shapes>
float ShapeResolver<Shapes...>::get_max_deriv() {
	float vals[sizeof...(Shapes)] = {Shapes::max_deriv()...};
	return *std::max_element(vals, vals+sizeof...(Shapes));
}

template <typename ... Shapes>
template <typename Shape>
Blocktype ShapeResolver<Shapes...>::gen_func(ShapeContext* ctx, ivec3 globalpos, int scale) {
	float val = Shape::gen_value(ctx, vec3(globalpos) + float(scale)/2);
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
Blocktype ShapeResolver<Shapes...>::gen_block(NodeView node, ShapeContext* ctx) {
	Blocktype val = gen_func<FirstShape>(ctx, node.globalpos, node.scale);
	if (val == BLOCK_NULL) {
		return gen_block<SecondShape,OtherShapes...>(node, ctx);
	} else {
		return gen_block<FirstShape,SecondShape,OtherShapes...>(node, val);
	}
}

template <typename ... Shapes>
template <typename Shape>
Blocktype ShapeResolver<Shapes...>::gen_block(NodeView node, ShapeContext* ctx) {
	Blocktype val = gen_func<Shape>(ctx, node.globalpos, node.scale);
	if (val == BLOCK_NULL) {
		val = 0;
	}
	return gen_block<Shape>(node, val);
}

template <typename ... Shapes>
template <typename ... OtherShapes>
Blocktype ShapeResolver<Shapes...>::gen_block(NodeView node) {
	ShapeContext context = get_context(vec3(node.globalpos) + float(node.scale)/2);
	return gen_block<OtherShapes...>(node, &context);
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
	double start = getTime();
	gen_block<Shapes...>(node);
	cout << getTime() - start << " time " << node.globalpos << endl;
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







template <int i, typename Other>
struct OtherShape : public Other {
	static constexpr int index = i;
};

template <typename Shape, int value>
struct SolidType : public Shape {
	static Blocktype block_val() {
		return value;
	}
};

template <typename Shape, int x, int y, int z>
struct Shift : public Shape {
	static float gen_value(ShapeContext* ctx, vec3 pos) {
		return Shape::gen_value(ctx, pos - vec3(x,y,z));
	}
};

template <typename Shape, int x, int y, int z>
struct Scale : public Shape {
	static float gen_value(ShapeContext* ctx, vec3 pos) {
		return Shape::gen_value(ctx, pos * vec3(x,y,z));
	}
	
	static float max_deriv() {
		return Shape::max_deriv() * std::max(x, std::max(y, z));
	}
};

template <typename ... Shapes>
struct Add {
	static float gen_value(ShapeContext* ctx, vec3 pos) {
		return (Shapes::gen_value(ctx, pos) + ...);
	}
	
	static float max_deriv() {
		return (Shapes::max_deriv() + ...);
	}
};

template <typename Shape1, int num, int denom = 1>
struct AddVal : public Shape1 {
	static constexpr float val = float(num) / denom;
	static float gen_value(ShapeContext* ctx, vec3 pos) {
		return Shape1::gen_value(ctx, pos) + val;
	}
};

template <typename Shape1, int num, int denom = 1>
struct MulVal : public Shape1 {
	static constexpr float val = float(num) / denom;
	static float gen_value(ShapeContext* ctx, vec3 pos) {
		return Shape1::gen_value(ctx, pos) * val;
	}
	
	static float max_deriv() {
		return Shape1::max_deriv() * std::abs(val);
	}
};






template <typename Shape, int num_falloff, int denom_falloff = 1>
struct HeightFalloff : public Shape {
	static float gen_value(ShapeContext* ctx, vec3 pos) {
		return Shape::gen_value(ctx, pos) - pos.y * num_falloff / denom_falloff;
	}
	
	static float max_deriv() {
		return Shape::max_deriv() + num_falloff / denom_falloff;
	}
};

template <int scale, int layer>
struct Perlin2d {
	static float gen_value(ShapeContext* ctx, vec3 pos) {
		return perlin2d(vec2(pos.x, pos.z) / float(scale), ctx->seed, layer) * scale;
	}
	
	static float max_deriv() {
		return 1.5f;
	}
};

template <int max_scale, int divider, int layer>
struct FractalPerlin2d {
	static float gen_value(ShapeContext* ctx, vec3 pos) {
		float val = 0;
	  int i = 0;
		int scale = max_scale;
	  while (scale > 1) {
	    val += perlin2d(vec2(pos.x, pos.z) / float(scale), ctx->seed, layer*100 + i) * scale;
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
	static float gen_value(ShapeContext* ctx, vec3 pos) {
		return perlin3d(pos / float(scale) + randvec3(ctx->seed, layer, 143, 211, 566), ctx->seed, layer) * scale * 2;
	}
	
	static float max_deriv() {
		return 1.5f;
	}
};

template <int max_scale, int divider, int layer>
struct FractalPerlin3d {
	static float gen_value(ShapeContext* ctx, vec3 pos) {
		// return perlin3d(pos / float(max_scale), ctx->seed, layer) * max_scale;
		float val = 0;
	  int i = 0;
		int scale = max_scale;
	  while (scale > 1) {
	    val += perlin3d(pos / float(scale), ctx->seed, layer*100 + i) * scale;
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
	static float gen_value(ShapeContext* ctx, vec3 pos) {
		return scale - std::abs(Perlin3d<scale, layer>::gen_value(ctx, pos));
	}
};








template <int height>
struct FlatTerrain {
	static float gen_value(ShapeContext* ctx, vec3 pos) {
		return height - pos.y;
	}
	
	static float max_deriv() {
		return 1;
	}
};

template <int height, int slope_numer, int slope_denom = 1>
struct SlopedTerrain {
	static float gen_value(ShapeContext* ctx, vec3 pos) {
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
	SolidType<LandLevel, 3>//,
	// SolidType<Shift<AddVal<OtherShape<0,LandLevel>, -3>, 0,2,0>, 1>,
	// SolidType<Shift<AddVal<OtherShape<0,LandLevel>, -5>, 0,5,0>, 2>
>;
EXPORT_PLUGIN_TEMPLATE(TestWorld);



class NullDecorator : public TerrainDecorator {
	PLUGIN(NullDecorator);
public:
	using TerrainDecorator::TerrainDecorator;
	virtual void decorate_chunk(NodeView node) {}
};

EXPORT_PLUGIN(NullDecorator);
