#pragma once
#include <stb_image.h>
#include "TextureManager.h"
#include "Camera.h"

enum LIGHT_TYPE { POINT_LIGHT, PARALELL_LIGHT };

float light_vertices[] = {
				-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
				 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
};

class Light: public Object
{
public:
	const char* srcLight_vertex_shader = "#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"layout(location = 1) in vec2 aTexCoord;\n"
		"out vec2 TexCoord;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
		"	TexCoord = aTexCoord;\n"
		"}";

	const char* srcLight_fragment_shader = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoord;\n"
		"uniform sampler2D myTexture;\n"
		"void main()\n"
		"{\n"
		"	FragColor = texture(myTexture, TexCoord);\n"
		"}\n";
public:
	Shader shader;
	// color of light
	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	// ambient intensity
	float ambientFactor = 0.1;
	// diffuse intensity
	float diffuseFactor = 1.0;
	// specular intensity
	float specularFactor = 10;
	// reflection strength
	float shininess = 32;
	// vertex array object id
	unsigned int VAO;
	// vertex buffer object id
	unsigned int VBO;
	// texture id
	unsigned int texture;
	// path of texture
	const char* pointLightIcon = "Resources/Icons/bulb.jpg";
	const char* paralLightIcon = "Resources/Icons/paral.jpg";
	// status
	bool visible = false;
	std::string status = "Invisible";

	Light(){}
	Light(glm::vec3 position, glm::vec3 lightColor, LIGHT_TYPE type):Object() {
		// parameter initialize
		this->transform.position = position;
		this->lightColor = lightColor;
		this->name = "Light";
		
		// initliaze shader
		this->shader = Shader(srcLight_vertex_shader, srcLight_fragment_shader);

		texture = TextureManager::getInstance()->load(this->pointLightIcon);

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(light_vertices), light_vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}
	~Light() {
		// deallocate
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}


	void render(Camera camera) {

		glm::vec3 position = this->transform.position;
		glm::vec3 rotation = this->transform.rotation;
		glm::vec3 scale = this->transform.scale;

		// DRAW
		// bind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		// create Transform
		// identity matrix
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		// transformation
		// translate to position
		model = glm::translate(model, glm::vec3(position[0], position[1], position[2]));
		// rotate around x-axis
		model = glm::rotate(model, glm::radians(rotation[0]), glm::vec3(1.0f, 0.0f, 0.0f));
		// rotate around y-axis
		model = glm::rotate(model, glm::radians(rotation[1]), glm::vec3(0.0f, 1.0f, 0.0f));
		// rotate around z-axis
		model = glm::rotate(model, glm::radians(rotation[2]), glm::vec3(0.0f, 0.0f, 1.0f));
		// scaling
		model = glm::scale(model, glm::vec3(scale[0], scale[1], scale[2]));
		// view
		view = glm::lookAt(camera.transform.position, -camera.transform.forward + camera.transform.position, camera.transform.up);
		// projection
		// projection = glm::perspective(glm::radians(100.0f), (float)WINDOW_WIDTH * 2.0f / (float)WINDOW_HEIGHT,  0.1f, 10.0f);
		projection = glm::perspective(glm::radians(camera.fovy), camera.aspect, camera.zNear, camera.zFar);
		// pass to shader
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		// enable the shader program
		shader.use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
};

