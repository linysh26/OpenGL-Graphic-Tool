#pragma once
#include <stb_image.h>
#include "Object.h"
#include "Camera.h"
#include "Light.h"

GLfloat planeVertices[] = {
	// Positions          // Normals         // Texture Coords
	25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
	-25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
	-25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

	25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
	25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f,
	-25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f
};


class Plane: public Object
{
private:
	// vertex array object id
	unsigned int VAO;
	// vertex buffer object id
	unsigned int VBO;
public:
	Plane() {}
	Plane(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
		this->transform.position = position;
		this->transform.rotation = rotation;
		this->transform.scale = scale;
	}
	~Plane() {}

	// render depth map
	void render(Shader shader) {
		glm::vec3 position = this->transform.position;
		glm::vec3 rotation = this->transform.rotation;
		glm::vec3 scale = this->transform.scale;

		unsigned int VAO;
		glGenVertexArrays(1, &VAO);
		unsigned int VBO;
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		// transformation
		// identity matrix
		glm::mat4 model = glm::mat4(1.0f);
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
		shader.setMat4("model", model);
		// enable the shader program
		shader.use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render with texture
	void render(Camera camera, Light light, Shader shader) {

		glm::vec3 position = this->transform.position;
		glm::vec3 rotation = this->transform.rotation;
		glm::vec3 scale = this->transform.scale;

		unsigned int VAO;
		glGenVertexArrays(1, &VAO);
		unsigned int VBO;
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		// DRAW
		shader.use();
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
		// view
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		// light properties
		shader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		shader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
		shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		shader.setVec3("viewPos", camera.transform.position);
		shader.setVec3("light.position", light.transform.position);
		shader.setVec3("light.color", light.lightColor);
		shader.setFloat("material.shininess", light.shininess);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
};

