#pragma once
#include <string>
#include "Shader.h"
#include "Camera.h"
#include "Light.h"



// define three kind of vertices
static enum VERTICES_ATRIBUTE_TYPE { POSITION, POSITION_COLOR, POSITION_TEXTURE, POSITION_COLOR_TEXTURE, POSITION_NORMAL };


class GameObject
{

public:
	// the name of the game object
	std::string name;

	Shader shader;

	// @dev define a structure of transform, which contains the position, rotation and scale
	// of the object, and also the three axis of the object's object space and world space.
	typedef struct Transform {
		// object's transform
		glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
		// object space axis
		glm::vec3 forward = { 0.0f, 0.0f, 1.0f };
		// positive direction of y-axis
		glm::vec3 up = { 0.0f, 1.0f, 0.0f };
		// positive direction of x-axis
		glm::vec3 right = { -1.0f, 0.0f, 0.0f };
		// world space axis ident vector
		glm::vec3 x = { 1.0f, 0.0f, 0.0f };
		glm::vec3 y = { 0.0f, 1.0f, 0.0f };
		glm::vec3 z = { 0.0f, 0.0f, 1.0f };

		glm::mat4 model;
	}Transform;
	// position, rotation and scale of the object
	Transform transform;

	GameObject() {}
	~GameObject() {}

	// @dev Rotate the object around the given axis with given angle
	// @param axis The axis to rotate around
	// @param angle The angle to rotate around the given axis
	void rotate(glm::vec3 axis, float angle) {
		glm::mat4 ident;
		ident = glm::rotate(ident, angle, axis);
		transform.model = ident * transform.model;
	}

	// @dev Translate from current position with given direction and distance
	// @param translation The translation vector
	void translate(glm::vec3 translation) {
		this->transform.position = this->transform.position + translation;
	}

	// @dev let the positive direction of object's z-axis point to a position in the world space
	// @param target The position we are going to look at
	void lookAt(glm::vec3 target) {
		// do something
	}

	virtual void render(Camera camera, Light light) {}
};

