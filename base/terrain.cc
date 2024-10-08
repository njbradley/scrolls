#include "terrain.h"

#include "blocks.h"
#include "blockdata.h"
#include "blockiter.h"

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

unsigned char permu_table[] = { 151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
	151,160,137,91,90,15
};

unsigned char permu(unsigned char val) {
	return permu_table[val];
}

int permu4(int seed, int a, int b, int c, int d) {
	return permu(permu(permu(permu(permu(d) + c) + b) + a) + seed);
}

int permu4(int seed, ivec3 pos, int d) {
	return permu4(seed, pos.x, pos.y, pos.z, d);
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







int fasthash(int seed, int xPrimed, int yPrimed) {
  int hash = seed ^ xPrimed ^ yPrimed;

  hash *= 0x27d4eb2d;
  return hash;
}

int fasthash(int seed, int xPrimed, int yPrimed, int zPrimed) {
  int hash = seed ^ xPrimed ^ yPrimed ^ zPrimed;

  hash *= 0x27d4eb2d;
  return hash;
}

float gradient_table3d[] = {
    0, 1, 1, 0,  0,-1, 1, 0,  0, 1,-1, 0,  0,-1,-1, 0,
    1, 0, 1, 0, -1, 0, 1, 0,  1, 0,-1, 0, -1, 0,-1, 0,
    1, 1, 0, 0, -1, 1, 0, 0,  1,-1, 0, 0, -1,-1, 0, 0,
    0, 1, 1, 0,  0,-1, 1, 0,  0, 1,-1, 0,  0,-1,-1, 0,
    1, 0, 1, 0, -1, 0, 1, 0,  1, 0,-1, 0, -1, 0,-1, 0,
    1, 1, 0, 0, -1, 1, 0, 0,  1,-1, 0, 0, -1,-1, 0, 0,
    0, 1, 1, 0,  0,-1, 1, 0,  0, 1,-1, 0,  0,-1,-1, 0,
    1, 0, 1, 0, -1, 0, 1, 0,  1, 0,-1, 0, -1, 0,-1, 0,
    1, 1, 0, 0, -1, 1, 0, 0,  1,-1, 0, 0, -1,-1, 0, 0,
    0, 1, 1, 0,  0,-1, 1, 0,  0, 1,-1, 0,  0,-1,-1, 0,
    1, 0, 1, 0, -1, 0, 1, 0,  1, 0,-1, 0, -1, 0,-1, 0,
    1, 1, 0, 0, -1, 1, 0, 0,  1,-1, 0, 0, -1,-1, 0, 0,
    0, 1, 1, 0,  0,-1, 1, 0,  0, 1,-1, 0,  0,-1,-1, 0,
    1, 0, 1, 0, -1, 0, 1, 0,  1, 0,-1, 0, -1, 0,-1, 0,
    1, 1, 0, 0, -1, 1, 0, 0,  1,-1, 0, 0, -1,-1, 0, 0,
    1, 1, 0, 0,  0,-1, 1, 0, -1, 1, 0, 0,  0,-1,-1, 0
};

float gradient_table2d[] = {
    0.130526192220052f, 0.99144486137381f, 0.38268343236509f, 0.923879532511287f, 0.608761429008721f, 0.793353340291235f, 0.793353340291235f, 0.608761429008721f,
    0.923879532511287f, 0.38268343236509f, 0.99144486137381f, 0.130526192220051f, 0.99144486137381f, -0.130526192220051f, 0.923879532511287f, -0.38268343236509f,
    0.793353340291235f, -0.60876142900872f, 0.608761429008721f, -0.793353340291235f, 0.38268343236509f, -0.923879532511287f, 0.130526192220052f, -0.99144486137381f,
    -0.130526192220052f, -0.99144486137381f, -0.38268343236509f, -0.923879532511287f, -0.608761429008721f, -0.793353340291235f, -0.793353340291235f, -0.608761429008721f,
    -0.923879532511287f, -0.38268343236509f, -0.99144486137381f, -0.130526192220052f, -0.99144486137381f, 0.130526192220051f, -0.923879532511287f, 0.38268343236509f,
    -0.793353340291235f, 0.608761429008721f, -0.608761429008721f, 0.793353340291235f, -0.38268343236509f, 0.923879532511287f, -0.130526192220052f, 0.99144486137381f,
    0.130526192220052f, 0.99144486137381f, 0.38268343236509f, 0.923879532511287f, 0.608761429008721f, 0.793353340291235f, 0.793353340291235f, 0.608761429008721f,
    0.923879532511287f, 0.38268343236509f, 0.99144486137381f, 0.130526192220051f, 0.99144486137381f, -0.130526192220051f, 0.923879532511287f, -0.38268343236509f,
    0.793353340291235f, -0.60876142900872f, 0.608761429008721f, -0.793353340291235f, 0.38268343236509f, -0.923879532511287f, 0.130526192220052f, -0.99144486137381f,
    -0.130526192220052f, -0.99144486137381f, -0.38268343236509f, -0.923879532511287f, -0.608761429008721f, -0.793353340291235f, -0.793353340291235f, -0.608761429008721f,
    -0.923879532511287f, -0.38268343236509f, -0.99144486137381f, -0.130526192220052f, -0.99144486137381f, 0.130526192220051f, -0.923879532511287f, 0.38268343236509f,
    -0.793353340291235f, 0.608761429008721f, -0.608761429008721f, 0.793353340291235f, -0.38268343236509f, 0.923879532511287f, -0.130526192220052f, 0.99144486137381f,
    0.130526192220052f, 0.99144486137381f, 0.38268343236509f, 0.923879532511287f, 0.608761429008721f, 0.793353340291235f, 0.793353340291235f, 0.608761429008721f,
    0.923879532511287f, 0.38268343236509f, 0.99144486137381f, 0.130526192220051f, 0.99144486137381f, -0.130526192220051f, 0.923879532511287f, -0.38268343236509f,
    0.793353340291235f, -0.60876142900872f, 0.608761429008721f, -0.793353340291235f, 0.38268343236509f, -0.923879532511287f, 0.130526192220052f, -0.99144486137381f,
    -0.130526192220052f, -0.99144486137381f, -0.38268343236509f, -0.923879532511287f, -0.608761429008721f, -0.793353340291235f, -0.793353340291235f, -0.608761429008721f,
    -0.923879532511287f, -0.38268343236509f, -0.99144486137381f, -0.130526192220052f, -0.99144486137381f, 0.130526192220051f, -0.923879532511287f, 0.38268343236509f,
    -0.793353340291235f, 0.608761429008721f, -0.608761429008721f, 0.793353340291235f, -0.38268343236509f, 0.923879532511287f, -0.130526192220052f, 0.99144486137381f,
    0.130526192220052f, 0.99144486137381f, 0.38268343236509f, 0.923879532511287f, 0.608761429008721f, 0.793353340291235f, 0.793353340291235f, 0.608761429008721f,
    0.923879532511287f, 0.38268343236509f, 0.99144486137381f, 0.130526192220051f, 0.99144486137381f, -0.130526192220051f, 0.923879532511287f, -0.38268343236509f,
    0.793353340291235f, -0.60876142900872f, 0.608761429008721f, -0.793353340291235f, 0.38268343236509f, -0.923879532511287f, 0.130526192220052f, -0.99144486137381f,
    -0.130526192220052f, -0.99144486137381f, -0.38268343236509f, -0.923879532511287f, -0.608761429008721f, -0.793353340291235f, -0.793353340291235f, -0.608761429008721f,
    -0.923879532511287f, -0.38268343236509f, -0.99144486137381f, -0.130526192220052f, -0.99144486137381f, 0.130526192220051f, -0.923879532511287f, 0.38268343236509f,
    -0.793353340291235f, 0.608761429008721f, -0.608761429008721f, 0.793353340291235f, -0.38268343236509f, 0.923879532511287f, -0.130526192220052f, 0.99144486137381f,
    0.130526192220052f, 0.99144486137381f, 0.38268343236509f, 0.923879532511287f, 0.608761429008721f, 0.793353340291235f, 0.793353340291235f, 0.608761429008721f,
    0.923879532511287f, 0.38268343236509f, 0.99144486137381f, 0.130526192220051f, 0.99144486137381f, -0.130526192220051f, 0.923879532511287f, -0.38268343236509f,
    0.793353340291235f, -0.60876142900872f, 0.608761429008721f, -0.793353340291235f, 0.38268343236509f, -0.923879532511287f, 0.130526192220052f, -0.99144486137381f,
    -0.130526192220052f, -0.99144486137381f, -0.38268343236509f, -0.923879532511287f, -0.608761429008721f, -0.793353340291235f, -0.793353340291235f, -0.608761429008721f,
    -0.923879532511287f, -0.38268343236509f, -0.99144486137381f, -0.130526192220052f, -0.99144486137381f, 0.130526192220051f, -0.923879532511287f, 0.38268343236509f,
    -0.793353340291235f, 0.608761429008721f, -0.608761429008721f, 0.793353340291235f, -0.38268343236509f, 0.923879532511287f, -0.130526192220052f, 0.99144486137381f,
    0.38268343236509f, 0.923879532511287f, 0.923879532511287f, 0.38268343236509f, 0.923879532511287f, -0.38268343236509f, 0.38268343236509f, -0.923879532511287f,
    -0.38268343236509f, -0.923879532511287f, -0.923879532511287f, -0.38268343236509f, -0.923879532511287f, 0.38268343236509f, -0.38268343236509f, 0.923879532511287f,
};

const int PrimeX = 501125321;
const int PrimeY = 1136930381;
const int PrimeZ = 1720413743;

int fastfloor(float f) { return f >= 0 ? (int)f : (int)f - 1; }
float lerp(float a, float b, float t) { return a + t * (b - a); }
float interp_hermite(float t) { return t * t * (3 - 2 * t); }
float interp_quintic(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

float grad_coord(int seed, int xPrimed, int yPrimed, int zPrimed, float xd, float yd, float zd) {
  int hash = fasthash(seed, xPrimed, yPrimed, zPrimed);
  hash ^= hash >> 15;
  hash &= 63 << 2;

  float xg = gradient_table3d[hash];
  float yg = gradient_table3d[hash | 1];
  float zg = gradient_table3d[hash | 2];

  return xd * xg + yd * yg + zd * zg;
}

float grad_coord(int seed, int xPrimed, int yPrimed, float xd, float yd) {
  int hash = fasthash(seed, xPrimed, yPrimed);
  hash ^= hash >> 15;
  hash &= 127 << 1;

  float xg = gradient_table2d[hash];
  float yg = gradient_table2d[hash | 1];

  return xd * xg + yd * yg;
}

float perlin3d(int seed, float x, float y, float z) {
  int x0 = fastfloor(x);
  int y0 = fastfloor(y);
  int z0 = fastfloor(z);

  float xd0 = (float)(x - x0);
  float yd0 = (float)(y - y0);
  float zd0 = (float)(z - z0);
  float xd1 = xd0 - 1;
  float yd1 = yd0 - 1;
  float zd1 = zd0 - 1;

  float xs = interp_quintic(xd0);
  float ys = interp_quintic(yd0);
  float zs = interp_quintic(zd0);

  x0 *= PrimeX;
  y0 *= PrimeY;
  z0 *= PrimeZ;
  int x1 = x0 + PrimeX;
  int y1 = y0 + PrimeY;
  int z1 = z0 + PrimeZ;

  float xf00 = lerp(grad_coord(seed, x0, y0, z0, xd0, yd0, zd0), grad_coord(seed, x1, y0, z0, xd1, yd0, zd0), xs);
  float xf10 = lerp(grad_coord(seed, x0, y1, z0, xd0, yd1, zd0), grad_coord(seed, x1, y1, z0, xd1, yd1, zd0), xs);
  float xf01 = lerp(grad_coord(seed, x0, y0, z1, xd0, yd0, zd1), grad_coord(seed, x1, y0, z1, xd1, yd0, zd1), xs);
  float xf11 = lerp(grad_coord(seed, x0, y1, z1, xd0, yd1, zd1), grad_coord(seed, x1, y1, z1, xd1, yd1, zd1), xs);

  float yf0 = lerp(xf00, xf10, ys);
  float yf1 = lerp(xf01, xf11, ys);

  return lerp(yf0, yf1, zs) * 0.964921414852142333984375f;
}

float perlin2d(int seed, float x, float y) {
  int x0 = fastfloor(x);
  int y0 = fastfloor(y);

  float xd0 = (float)(x - x0);
  float yd0 = (float)(y - y0);
  float xd1 = xd0 - 1;
  float yd1 = yd0 - 1;

  float xs = interp_quintic(xd0);
  float ys = interp_quintic(yd0);

  x0 *= PrimeX;
  y0 *= PrimeY;
  int x1 = x0 + PrimeX;
  int y1 = y0 + PrimeY;

  float xf0 = lerp(grad_coord(seed, x0, y0, xd0, yd0), grad_coord(seed, x1, y0, xd1, yd0), xs);
  float xf1 = lerp(grad_coord(seed, x0, y1, xd0, yd1), grad_coord(seed, x1, y1, xd1, yd1), xs);

  return lerp(xf0, xf1, ys) * 1.4247691104677813f;
}






















TerrainValue::TerrainValue(): value(0), deriv(0) {
	
}

TerrainValue::TerrainValue(float newvalue, float newderiv): value(newvalue), deriv(newderiv) {
	
}

TerrainValue TerrainValue::operator+(const TerrainValue& other) const {
	return TerrainValue(value + other.value, deriv + other.deriv);
}
TerrainValue TerrainValue::operator-(const TerrainValue& other) const {
	return TerrainValue(value - other.value, deriv + other.deriv);
}
TerrainValue TerrainValue::operator*(const TerrainValue& other) const {
	float dist1 = std::abs(value / deriv), dist2 = std::abs(other.value / other.deriv);
	float newval = value * other.value;
	float newdist = std::min(dist1, dist2);
	return TerrainValue(newval, std::abs(newdist * newval));
	//return TerrainValue(value * other.value, deriv*std::abs(other.value) + std::abs(value)*other.deriv);
}

TerrainValue TerrainValue::operator+(float val) const {
	return TerrainValue(value + val, deriv);
}
TerrainValue TerrainValue::operator-(float val) const {
	return TerrainValue(value - val, deriv);
}
TerrainValue TerrainValue::operator*(float val) const {
	return TerrainValue(value * val, deriv * std::abs(val));
}
TerrainValue TerrainValue::operator/(float val) const {
	return TerrainValue(value / val, deriv / std::abs(val));
}

TerrainValue& TerrainValue::operator+=(const TerrainValue& other) {
	value += other.value;
	deriv += other.deriv;
	return *this;
}
TerrainValue& TerrainValue::operator-=(const TerrainValue& other) {
	value -= other.value;
	deriv += other.deriv;
	return *this;
}
TerrainValue& TerrainValue::operator*=(const TerrainValue& other) {
	value *= other.value;
	deriv = deriv*std::abs(other.value) + std::abs(value)*other.deriv;
	return *this;
}

TerrainValue& TerrainValue::operator+=(float val) {
	value += val;
	return *this;
}
TerrainValue& TerrainValue::operator-=(float val) {
	value -= val;
	return *this;
}
TerrainValue& TerrainValue::operator*=(float val) {
	value *= val;
	deriv *= std::abs(val);
	return *this;
}
TerrainValue& TerrainValue::operator/=(float val) {
	value /= val;
	deriv /= std::abs(val);
	return *this;
}


bool TerrainValue::operator<(const TerrainValue& other) const {
	return value < other.value;
}
bool TerrainValue::operator>(const TerrainValue& other) const {
	return value > other.value;
}

TerrainValue TerrainValue::lerp(const TerrainValue& val1, const TerrainValue& val2, float amount) {
	return TerrainValue(val1.value * (1-amount) + val2.value * amount, val1.deriv * (1-amount) + val2.deriv * amount);
}

TerrainValue TerrainValue::min(const TerrainValue& val1, const TerrainValue& val2) {
	return TerrainValue(std::min(val1.value, val2.value), std::max(val1.deriv, val2.deriv));
	//if (val1.value / val1.deriv > val2.value / val2.deriv) {
	//	return val2;
	//} else {
	//	return val1;
	//}
}

TerrainValue TerrainValue::max(const TerrainValue& val1, const TerrainValue& val2) {
	return TerrainValue(std::max(val1.value, val2.value), std::max(val1.deriv, val2.deriv));
	//if (val1.value / val1.deriv < val2.value / val2.deriv) {
	//	return val2;
	//} else {
	//	return val1;
	//}
}

TerrainValue TerrainValue::abs(const TerrainValue& val) {
	return TerrainValue(std::abs(val.value), val.deriv);
}

TerrainValue default_falloff(const Layers* layers) {
	return TerrainValue(1,0);
}

ShapeValue::ShapeValue(const TerrainValue& terrvalue, BlockData* block, LayerFunc layergen, BiomeFunc biome):
TerrainValue(terrvalue), btype(block), layergen(layergen), biome(biome), falloff(default_falloff) {
	
}

ShapeValue::ShapeValue(const TerrainValue& terrvalue, ShapeFunc falloff, BlockData* block, LayerFunc layergen, BiomeFunc biome):
TerrainValue(terrvalue), btype(block), layergen(layergen), biome(biome), falloff(falloff) {
	
}

bool ShapeValue::operator<(const ShapeValue& other) const {
	return value < other.value;
}
bool ShapeValue::operator>(const ShapeValue& other) const {
	return value > other.value;
}

bool ShapeValue::needs_split(int scale) {
	return std::abs(value) < (scale-1) * 1.73f * deriv;
}



TerrainValue perlin2d(int seed, vec3 pos, int scale, int height, int layer = 0) {
	return TerrainValue(
		perlin2d(seed + layer, pos.x / float(scale), pos.z / float(scale)) * height / 2,
		1.5f * height / scale
	);
}


TerrainValue perlin3d(int seed, vec3 pos, int scale, int height, int layer) {
	return TerrainValue(
		perlin3d(seed + layer, pos.x / float(scale), pos.y / float(scale), pos.z / float(scale)) * height / 2,
		1.5f * height / scale
	);
}







LerpLayerGen::LerpLayerGen(TerrainContext* ctx, LayerFunc shape, vec3 samplepoint, float scale): position(samplepoint), scale(scale) {
	float max_val[Layers::num_layers];
	std::fill(max_val, max_val+Layers::num_layers, -999999);
	float min_val[Layers::num_layers];
	std::fill(min_val, min_val+Layers::num_layers, 999999);
	for (int i = 0; i < 8; i ++) {
		vec3 off (i/4, i/2%2, i%2);
		off *= scale;
		Layers tmplayers;
		shape(ctx, &tmplayers, samplepoint + off);
		for (int j = 0; j < Layers::num_layers; j ++) {
			float val = tmplayers.layers()[j].value;
			samples[i].values[j] = val;
			max_val[j] = std::max(max_val[j], val);
			min_val[j] = std::min(min_val[j], val);
		}
		//cout << samplepoint + off << ' ' << samples[i].ground_level.value << endl;
	}

	for (int i = 0; i < Layers::num_layers; i ++) {
		max_deriv[i] = (max_val[i] - min_val[i]) / scale;
	}
}

void LerpLayerGen::add_value(Layers* outlayers, vec3 pos) {
	for (int i = 0; i < Layers::num_layers; i ++) {
		vec3 off = (pos - position) / scale;
		
		float x0 = lerp(samples[0].values[i], samples[4].values[i], off.x);
		float x1 = lerp(samples[1].values[i], samples[5].values[i], off.x);
		float x2 = lerp(samples[2].values[i], samples[6].values[i], off.x);
		float x3 = lerp(samples[3].values[i], samples[7].values[i], off.x);

		float y0 = lerp(x0, x2, off.y);
		float y1 = lerp(x1, x3, off.y);
		
		outlayers->layers()[i] += TerrainValue(lerp(y0, y1, off.z), max_deriv[i]);
	}
}

template <LayerFunc func1, LayerFunc func2>
void add_layergen(TerrainContext* ctx, Layers* outlayers, vec3 pos) {
	func1(ctx, outlayers, pos);
	func2(ctx, outlayers, pos);
}


BiomeResult mountain_biome(TerrainContext* ctx, const Layers* layers, ivec3 pos, int scale);
BiomeResult generate_biome_shapes(vector<ShapeValue> shapes, ivec3 pos, int scale) {
	for (ShapeValue val : shapes) {
		if (val.needs_split(scale) or val.value < 0) {
			BiomeResult result;
			result.blocktype = val.btype;
			result.nextlayergen = val.layergen;
			result.nextbiome = val.biome;
			result.needs_split = val.needs_split(scale);
			result.falloff = val.falloff;
			return result;
		}
	}
	BiomeResult result;
	result.blocktype = nullptr;
	result.nextlayergen = nullptr;
	result.nextbiome = nullptr;
	result.needs_split = false;
	result.falloff = default_falloff;
	return result;
}



BiomeResult mountain_biome(TerrainContext* ctx, const Layers* layers, ivec3 pos, int scale) {
	return generate_biome_shapes({
		//ShapeValue(TerrainValue::max(layers->ground_level, perlin2d(ctx->seed, pos, 32, 8, 4) + TerrainValue(50-pos.y, 1)), &blocktypes::snow),
		ShapeValue(layers->stone_level, &blocktypes::stone),
		ShapeValue(layers->ground_level, &blocktypes::snow)
	}, pos, scale);
}

void mountain_layergen(TerrainContext* ctx, Layers* outlayers, vec3 pos) {
	TerrainValue falloff = TerrainValue::min(ctx->falloff(outlayers) * -8.0f, TerrainValue(1,0));
	outlayers->ground_level -= (perlin2d(ctx->seed, pos, 64, 64, 2) + 32) * falloff;
	outlayers->stone_level -= (perlin2d(ctx->seed, pos+vec3(0,10,0), 64, 64, 2) + 32) * falloff;
}



void plains_layergen(TerrainContext* ctx, Layers* layers, vec3 pos) {
	TerrainValue falloff = TerrainValue::min(ctx->falloff(layers) * -8.0f, TerrainValue(1,0));
	layers->ground_level += falloff * pos.y * 5;
	layers->stone_level += falloff * (pos.y + 10) * 5;
}

BiomeResult plains_biome(TerrainContext* ctx, const Layers* layers, ivec3 pos, int scale) {
	return generate_biome_shapes({
		ShapeValue(layers->stone_level, &blocktypes::stone),
		ShapeValue(layers->ground_level, &blocktypes::grass)
	}, pos, scale);
}

TerrainValue root_groundlevel(TerrainContext* ctx, Layers* layers, vec3 pos) {
	TerrainValue val = TerrainValue::abs(perlin2d(ctx->seed, pos, 64, 128, 5)) * -1.0f;
	TerrainValue val2 = TerrainValue::abs(perlin2d(ctx->seed, pos, 64, 128, 6)) * -1.0f;
	return TerrainValue(pos.y, 1) + TerrainValue::max(val, TerrainValue(64,0));
	return TerrainValue(pos.y, 1) + perlin2d(ctx->seed, pos, 64, 32, 5) - 16 + perlin2d(ctx->seed, pos, 31, 16, 6);
	return TerrainValue(pos.y, 1) + perlin2d(ctx->seed, pos, 64, 32, 5) - 16 + perlin3d(ctx->seed, pos, 65, 65, 3)
		+ perlin3d(ctx->seed, pos, 31, 31, 4) + perlin3d(ctx->seed, pos, 15, 15, 5);
}

void root_layergen(TerrainContext* ctx, Layers* outlayers, vec3 pos) {
	outlayers->temperature += perlin3d(ctx->seed, pos, 128, 2, 0) - pos.y * 0.01f;
	outlayers->humidity += perlin3d(ctx->seed, pos, 128, 2, 2);
	//outlayers->elevation += perlin2d(ctx->seed, pos, 128, 2, 1) - pos.y * 0.01f;

	outlayers->ground_level += perlin2d(ctx->seed, pos, 64, 64, 1) + TerrainValue(pos.y, 1); //root_groundlevel(ctx, outlayers, pos);
	//outlayers->stone_level += root_groundlevel(ctx, outlayers, pos+vec3(0,10,0)) - 10;
}

BiomeResult root_biome(TerrainContext* ctx, const Layers* layers, ivec3 pos, int scale) {
	//return generate_biome_shapes({ShapeValue(layers->ground_level, &blocktypes::dirt)}, pos, scale);
	TerrainValue ground_area = TerrainValue::abs(layers->ground_level) * -1.0f + 256.0f;
	
	ShapeFunc coldfunc = [] (const Layers* layers) {
		return layers->temperature + 0.33f;
	};
	ShapeFunc hotfunc = [] (const Layers* layers) {
		return layers->temperature * -1.0f + 0.33f;
	};

    /*
	ShapeFunc lowfunc = [] (const Layers* layers) {
		return layers->elevation + 0.33f;
	};
	ShapeFunc highfunc = [] (const Layers* layers) {
		return layers->elevation * -1.0f + 0.33f;
	};
    */
	
	return generate_biome_shapes({
		ShapeValue(layers->ground_level, &blocktypes::stone),
		//ShapeValue(highfunc(layers), &blocktypes::stone, &mountain_layergen, &mountain_biome),
		//ShapeValue(lowfunc(layers), &blocktypes::grass, &plains_layergen, &plains_biome)
		//ShapeValue(layers->stone_level, &blocktypes::stone),
		//ShapeValue(layers->ground_level, &blocktypes::dirt)
	}, pos, scale);
}

void zero_layergen(TerrainContext* ctx, Layers* layers, vec3 pos) {}

//template <LayerFunc layerfunc, 
//void interp_layergen(TerrainContext* ctx, Layers* layers, vec3 pos) {
	



LayerFunc BiomeGenerator::root_layergen = &::root_layergen;

BiomeFunc BiomeGenerator::root_biome = &::root_biome;

void BiomeGenerator::generate_chunk(NodeView node, int depth) {
	TerrainContext context;
	context.seed = seed;
	context.falloff = &default_falloff;
	LerpLayerGen initial_gen (&context, zero_layergen, node.position, node.scale);
	gen_node(node, &initial_gen, root_layergen, root_biome, depth, default_falloff);
}

BlockData* BiomeGenerator::gen_node(NodeView node, LerpLayerGen* prevlayergen, LayerFunc layergen, BiomeFunc biome, int depth, ShapeFunc falloff) {
	vec3 pos = node.position;
	pos += float(node.scale)/2;
	TerrainContext context;
	context.seed = seed;
	context.falloff = falloff;
	
	Layers layers;
	prevlayergen->add_value(&layers, pos);
	layergen(&context, &layers, pos);

	BiomeResult result = biome(&context, &layers, node.position, node.scale);
	//cout << result.falloff.value << "--------------" << endl;
	//cout << result.falloff.value << ' ' << (void*)biome << ' ' << (void*)&mountain_biome << endl;
	//result.nextlayergen = nullptr;
	//result.nextbiome = nullptr;
	
	if (result.needs_split) {
		node.split();
		
		BlockData* blocktype = gen_node(node.child(0), prevlayergen, layergen, biome, depth-1, falloff);
		for (int i = 1; i < BDIMS3; i ++) {
			BlockData* newtype = gen_node(node.child(i), prevlayergen, layergen, biome, depth-1, falloff);
			blocktype = (newtype == blocktype) ? blocktype : BLOCK_SPLIT;
		}

		if (blocktype != BLOCK_SPLIT) {
			node.join();
			node.set_block(new Block(blocktype));
		}

		return blocktype;
	} else if (result.nextlayergen != nullptr) {
		LerpLayerGen lerplayergen (&context, layergen, node.position, node.scale);
		if (result.nextbiome == nullptr) {
			return gen_node(node, &lerplayergen, result.nextlayergen, biome, depth, falloff);
		} else {
			return gen_node(node, &lerplayergen, result.nextlayergen, result.nextbiome, depth, result.falloff);
		}
	} else if (result.nextbiome != nullptr) {
		return gen_node(node, prevlayergen, layergen, result.nextbiome, depth, result.falloff);
	} else {
		node.set_block(new Block(result.blocktype));
		return result.blocktype;
	}
}

int BiomeGenerator::get_height(ivec3 pos) {
	return 0;
}




EXPORT_PLUGIN(BiomeGenerator);













DEFINE_PLUGIN(TerrainGenerator);

TerrainGenerator::TerrainGenerator(int newseed): seed(newseed) {
	
}



DEFINE_PLUGIN(TerrainDecorator);

TerrainDecorator::TerrainDecorator(int newseed): seed(newseed) {
	
}

/*

template <typename Layers, ShapeFunc ... Shapes>
void ShapeResolver<Layers,Shapes...>::generate_chunk(NodeView node, int depth) {
	
	// float max_deriv = 0;
	// for (int i = 0; i < 100000000; i ++) {
	// 	vec3 pos = randvec3(123435, i, 1, 3, 4) * 100.0f;
	// 	vec3 dir = randvec3(123456, i, 45, 3, 2) * randfloat(123232, i, 54, 32, 2);
	// 	float val1 = perlin3d(12345, pos.x, pos.y, pos.z);
	// 	float val2 = perlin3d(12345, pos.x + dir.x, pos.y + dir.y, pos.z + dir.z);
	// 	float deriv = std::abs(val1 - val2) / glm::length(dir);
	// 	if (deriv > max_deriv) {
	// 		max_deriv = deriv;
	// 	}
	// }
	// cout << max_deriv << " DERIV" << endl;
	
	// int scale = node.scale;
	// int depth = 0;
	// while (scale > min_scale) {
	// 	depth ++;
	// 	scale /= BDIMS;
	// }
	
	double start = getTime();
	ShapeFunc shapes[] = {Shapes...};
	gen_node(node, shapes, sizeof...(Shapes), depth);
	double time = getTime() - start;
	// cout << "generated " << node.position << " in " << time << endl;
}

template <typename Layers, ShapeFunc ... Shapes>
BlockData* ShapeResolver<Layers,Shapes...>::gen_node(NodeView node, ShapeFunc* shapes, int num_shapes, int max_depth) {
	node.reset_flag(Block::GENERATION_FLAG);
	
	if (node.haschildren()) {
		if (max_depth <= 0) {
			return 0;
		}
		BlockData* blocktype = gen_node(node.child(0), shapes, num_shapes, max_depth-1);
		for (int i = 1; i < BDIMS3; i ++) {
			if (node.child(i).test_flag(Block::GENERATION_FLAG)) {
				BlockData* newblock = gen_node(node.child(i), shapes, num_shapes, max_depth-1);
				blocktype = blocktype != newblock ? BLOCK_SPLIT : blocktype;
			}
		}
		return blocktype;
	}
	
	vec3 pos = vec3(node.position) + float(node.scale)/2;
	
	BlockData* blocktype;
	ShapeContext context (seed, pos, layers.layers, layers.size);
	ShapeValue value;
	int index;
	for (index = 0; index < num_shapes; index ++) {
		value = shapes[index](&context, pos);
		blocktype = value.blocktype(node.scale);
		
		if (blocktype != BLOCK_NULL) {
			break;
		}
	}
	
	if (blocktype == BLOCK_SPLIT) {
		// cout << "split " << endl;
		for (int i = index; i < num_shapes; i ++) {
			ShapeValue newvalue = shapes[i](&context, pos);
			if (newvalue.blocktype(node.scale) != BLOCK_SPLIT and newvalue.blocktype(node.scale) != BLOCK_NULL) {
				value = newvalue;
				break;
			}
			if (newvalue > value) {
				value = newvalue;
			}
		}
		
		if (max_depth > 0) {
			node.split();
			blocktype = gen_node(node.child(0), shapes + index, num_shapes - index, max_depth-1);
			for (int i = 1; i < BDIMS3; i ++) {
				BlockData* newblock = gen_node(node.child(i), shapes + index, num_shapes - index, max_depth-1);
				blocktype = blocktype != newblock ? BLOCK_SPLIT : blocktype;
			}
			if (blocktype == BLOCK_SPLIT) {
				// cout << " actual " << endl;
				blocktype = value.blocktype(0);
				node.set_last_pix(blocktype);
				return BLOCK_SPLIT;
			}
			// cout << " join" << endl;
			node.join();
		} else {
			blocktype = value.blocktype(0);
			node.set_flag(Block::GENERATION_FLAG);
		}
	}
	
	if (blocktype == BLOCK_NULL) {
		blocktype = 0;
	}
	
	// cout << "blocktype: " << blocktype << endl;
	node.set_block(new Block(blocktype));
	return blocktype;
}

template <typename Layers, ShapeFunc ... Shapes>
int ShapeResolver<Layers,Shapes...>::get_height(ivec3 ipos) {
	LayerFunc func = layers.layers[0];
	vec3 pos = ipos;
	TerrainContext context (seed, pos);
	TerrainValue val = func(&context, pos);
	int i = 0;
	while (std::abs(val.value) > 2 and i < 200) {
		pos.y += val.value / val.deriv;
		val = func(&context, pos);
		i ++;
	}
	return pos.y;
}









using UndefShapeFunc = TerrainValue (*) (ShapeContext* ctx, vec3 pos);

template <BlockData& btype, LayerFunc func>
ShapeValue set_blocktype(ShapeContext* ctx, vec3 pos) {
	return ShapeValue(func(ctx, pos), &btype);
}

template <BlockData& btype, UndefShapeFunc func>
ShapeValue set_blocktype(ShapeContext* ctx, vec3 pos) {
	return ShapeValue(func(ctx, pos), &btype);
}







template <int num_falloff, int denom_falloff = 1>
TerrainValue height_falloff(TerrainContext* ctx, vec3 pos) {
	return TerrainValue(
		-pos.y * num_falloff / denom_falloff,
		num_falloff / denom_falloff
	);
}




template <int scale, int height, int layer>
TerrainValue perlin2d(TerrainContext* ctx, vec3 pos) {
	return TerrainValue(
		perlin2d(ctx->seed + layer, pos.x / float(scale), pos.z / float(scale)) * height / 2,
		1.5f * height / scale
	);
}




template <int scale, int height, int layer>
TerrainValue perlin3d(TerrainContext* ctx, vec3 pos) {
	return TerrainValue(
		perlin3d(ctx->seed + layer, pos.x / float(scale), pos.y / float(scale), pos.z / float(scale)) * height / 2,
		1.5f * height / scale
	);
}



template <int height>
TerrainValue flat_terrain(TerrainContext* ctx, vec3 pos) {
	TerrainValue val (
		height - pos.y,
		1
	);
	return val;
}

template <int height, int slope_numer, int slope_denom = 1>
TerrainValue sloped_terrain(TerrainContext* ctx, vec3 pos) {
	return TerrainValue(
		height + pos.x * slope_numer / slope_denom - pos.y,
		std::max(std::abs((float)slope_numer/slope_denom), 1.0f)
	);
}

template <int index>
TerrainValue layer_ref(ShapeContext* ctx, vec3 pos) {
	return ctx->layer(index);
}




TerrainValue land_level(TerrainContext* ctx, vec3 pos) {
	// return perlin2d<64,64,1>(ctx,pos) + height_falloff<1>(ctx,pos);
	return perlin3d<64,64,0>(ctx,pos) + perlin3d<9,9,1>(ctx,pos)
		+ perlin3d<32,32,4>(ctx,pos) + perlin2d<512,256,9>(ctx,pos) + height_falloff<1>(ctx,pos);
}

template <int height>
TerrainValue shifted_land_level(TerrainContext* ctx, vec3 pos) {
	return land_level(ctx, pos - vec3(0,height,0));
}

ShapeValue grass_level(ShapeContext* ctx, vec3 pos) {
	return ShapeValue(
		ctx->layer(1) - 4,
		&blocktypes::grass
	);
}

ShapeValue dirt_level(ShapeContext* ctx, vec3 pos) {
	return ShapeValue(
		(ctx->layer(0) + ctx->layer(1)) / 2 - 2,
		&blocktypes::dirt
	);
}

//EXPORT_PLUGIN_TEMPLATE(ShapeResolver<
//	// LayerResolver<>,
//	LayerResolver<land_level, shifted_land_level<5>>,
//	// LayerResolver<flat_terrain<16>>,
//	// LayerResolver<sloped_terrain<16,1,3>>,
//	set_blocktype<blocktypes::stone,layer_ref<0>>,
//	// set_blocktype<1,layer_ref<1>>,
//	// set_blocktype<2,layer_ref<2>>
//	dirt_level,
//	grass_level
//	// set_blocktype<3,land_level>,
//	// set_blocktype<1,shifted_land_level<3>>,
//	// set_blocktype<2,shifted_land_level<5>>
//	// set_blocktype<1,land_level>
//>);


// using LandLevel = HeightFalloff<Add<Perlin3d<64,0>, Perlin3d<9,1>, RidgePerlin3d<32,4>, Perlin2d<512,9>>, 1>;
//
// using TestWorld = ShapeResolver<
// 	SolidType<LandLevel, 3>//,
// 	// SolidType<Shift<AddVal<OtherShape<0,LandLevel>, -3>, 0,2,0>, 1>,
// 	// SolidType<Shift<AddVal<OtherShape<0,LandLevel>, -5>, 0,5,0>, 2>
// >;
// EXPORT_PLUGIN_TEMPLATE(TestWorld);

int PARAM(min_scale_div) = 1;

struct SubDivGenerator : public TerrainGenerator {
	PLUGIN(SubDivGenerator);
	using TerrainGenerator::TerrainGenerator;
	
	void gen_node(NodeView node, int depth) {
		int score = 0;
		for (int x = -1; x <= 1; x ++) {
			for (int y = -1; y <= 1; y ++) {
				for (int z = -1; z <= 1; z ++) {
					if (x == 0 and y == 0 and z == 0) continue;
					ivec3 off (x,y,z);
					NodeView sidenode = node.get_global(node + off);
					if (sidenode.isvalid()) {
						if (sidenode.hasblock() and sidenode.block()->type == &blocktypes::stone) {
							score += sidenode.scale;
						}
					} else if (off == ivec3(0,-1,0)) {
						score += node.scale * 27;
					}
				}
			}
		}

		if (score > node.scale * (hash4(seed, node.position, node.scale) % 15 + 5)) {
			node.set_block(new Block(&blocktypes::dirt));
		} else if (score > 0 and node.scale > min_scale_div) {
			node.set_flag(Block::GENERATION_FLAG);
		} else {
			node.set_block(new Block(nullptr));
		}
	}
	
	
	virtual void generate_chunk(NodeView node, int depth) {
		node.set_flag(Block::GENERATION_FLAG);
		cout << "GENERATING __________" << endl;
		
		while (node.test_flag(Block::GENERATION_FLAG)) {
			cout << node.haschildren() << ' ' << node.position << ' ' << node.scale << endl;
			cout << "starting generation round " << endl;
			for (NodePtr curnode : NodePtr(node).iter<NodeIter>()) {
				cout << "brug" << endl;
				if (curnode.hasblock() and curnode.block()->type == &blocktypes::dirt) {
					cout << "bksdjflksd" << endl;
					curnode.block()->type = &blocktypes::stone;
				}
			}
			for (NodeView curnode : node.iter<FlagNodeIter>(Block::GENERATION_FLAG)) {
				cout << node.position << ' ' << node.scale << endl;
				curnode.reset_flag(Block::GENERATION_FLAG);
				if (curnode.haschildren()) continue;
				
				cout << " splitting " << curnode.position << ' ' << curnode.scale << endl;
				curnode.split();
				for (NodeView curcurnode : curnode.iter<ChildIter>()) {
					gen_node(curcurnode, depth);
				}
			}
		}
	}

	virtual int get_height(ivec3 pos) {
		return 0;
	}
};

//EXPORT_PLUGIN(SubDivGenerator);
*/



class NullDecorator : public TerrainDecorator {
	PLUGIN(NullDecorator);
public:
	using TerrainDecorator::TerrainDecorator;
	virtual void decorate_chunk(NodeView node) {}
};

EXPORT_PLUGIN(NullDecorator);
