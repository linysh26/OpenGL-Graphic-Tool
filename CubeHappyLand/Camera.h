#ifndef CAMERA_H
#define CAMERA_H

#include "Object.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma once
class Camera: public Object
{
public:
	// field of view
	float fovy = 45.0f;
	// aspect
	float aspect = 0.0f;
	// perspective cutoff
	float zNear = 1.0f;
	float zFar = 100.0f;
	// speed of moving the camera
	float speed = 0.05f;
public:
	Camera() {}
	Camera(float fovy, float zNear, float zFar) {
		this->fovy = fovy;
		this->zNear = zNear;
		this->zFar = zFar;
		this->transform.position = { 0.0f, 0.0f, 10.0f };
	}
	~Camera() {}
};


#endif