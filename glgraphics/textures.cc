#include "textures.h"

#include <fstream>


template <typename T>
void read_val(istream& ifile, T* ptr) {
	ifile.read((char*)ptr, sizeof(T));
}

uint8* load_bmp(string path, int* width_out, int* height_out, int* channels_out) {
	std::ifstream ifile (path);
	
	uint32 pixel_offset;
	ifile.seekg(10);
	read_val(ifile, &pixel_offset);
	
	uint32 height, width;
	uint16 pixel_bits;
	ifile.seekg(18);
	read_val(ifile, &width);
	read_val(ifile, &height);
	read_val(ifile, &pixel_bits);
	read_val(ifile, &pixel_bits);
	
	int channels = pixel_bits / 8;
	
	*height_out = height;
	*width_out = width;
	*channels_out = channels;
	
	// cout << " INFO " << pixel_offset << ' ' << height << ' ' << width << ' ' << channels << endl;
	
	uint8* data = new uint8[width * height * channels];
	
	ifile.seekg(pixel_offset);
	ifile.read((char*)data, width * height * channels);
	return data;
}


GLuint load_array(vector<string>& paths, int size) {
	
	uint8* data = new uint8[size*size*4*paths.size()];
	int offset = 0;
	for (string path : paths) {
		int width, height, channels;
		uint8* newdata = load_bmp(path, &width, &height, &channels);
		ASSERT(width == size and height == size and channels == 4);
		std::copy(newdata, newdata + (size*size*4), data + offset);
		offset += size*size*4;
		delete[] newdata;
	}
	
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0,GL_RGBA, size, size, paths.size(), 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
	
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	
	delete[] data;
	
	return textureID;
}
	
	
	
