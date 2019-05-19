#pragma once

class Texture
{
public:
	unsigned int textureId;
	Texture() {}
	Texture(unsigned int id) {
		this->textureId = id;
	}
	~Texture() {}
};

