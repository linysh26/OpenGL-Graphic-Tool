#pragma once
#include "Texture.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <string>

class TextureManager
{
private:
	TextureManager() {}
	~TextureManager() {
		// deallocate textures
		for (int i = 0; i < size; i++) {
			glDeleteTextures(1, &textures[i].textureId);
		}
		delete instance;
	}
	// textures
	Texture textures[10];
	int size = 0;
public:

	unsigned int load(const char* path) {

		unsigned int texture;
		// create texture
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// set texture wrapping parameters
		// for S direction (x direction)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		// for T direction (y direction)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// for minify
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// load image
		int width, height, nrChannels;
		// tell stb_image.h to flip the loaded texture's on the y-axis
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			std::cout << "Failed to load texture!" << std::endl;
		}
		stbi_image_free(data);
		
		this->textures[size] = Texture(texture);
		size++;

		return texture;
	}

	// makes TextureManager an instance
	static TextureManager* getInstance() {
		if (instance == NULL) {
			instance = new TextureManager();
		}
		return instance;
	}
private:
	static TextureManager* instance;
};

TextureManager* TextureManager::instance = NULL;