#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "ShaderCode.h"
#include "TextureManager.h"
#include "Camera.h"
#include "Light.h"
#include "Cube.h"
#include "Plane.h"

#define WINDOW_HEIGHT 1600
#define WINDOW_WIDTH 1600
#define DEFAULT_OPTION 0

#define PI 3.14

using namespace std;

// error callback
void error_callback(int error, const char* description);
// initialize callback function for adapting the view port size to new size of the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// check if some keys press
void processInput(GLFWwindow *window);
// mouse action callback
void processCursor(GLFWwindow *window);
// dividebackground into grid
void drawGrid(int rows, int cols, float* color, unsigned int& shaderProgram);
// draw line from v1 to v2 using Bresenhem
void myLineTo(float* v1, float* v2, float* color, unsigned int& shaderProgram);
// draw circle at center with radius using Bresenhem
void myCircleAt(float* center, float radius, float* color, unsigned int& shaderProgram);
// draw square with primitive GL_TRIANLE
void drawSquare2D(float xi_1, float yi_1, float edgeLength, float* color, unsigned int& shaderProgram);
// draw line with primitive GL_LINE
void drawLine(float* v1, float* v2, float* color, unsigned int& shaderProgram);
// draw circle with primitive GL_LINE_LOOP
void drawCircle(float* center, float radius, float* color, int count, unsigned int& shaderProgram);
// draw triangle with primitive GL_TRIANLE
void drawTriangle(float* v1, float* v2, float* v3, float* color, unsigned int& shaderProgram);

// mouse position
double xPos = 0;
double yPos = 0;
// imgui's status
bool isActive = true;
// user's option
int option = DEFAULT_OPTION;
// picker's color
float picker_color[3] = { 1.0f, 0.5f, 0.31f };
// grid color
float grid_color[3] = { 1.0f, 1.0f, 1.0f };
// line and circles' color
float draw_color[3] = { 0.0f, 0.0f, 1.0f };

// camera
Camera camera(45.0f, 1.0f, 100.0f);
int key_grave_state = GLFW_RELEASE;


// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{

	// ************************************* OpenGL window initialization ********************************
	// set callback function
	glfwSetErrorCallback(error_callback);
	// init
	if (!glfwInit()) {
		// initialization failed!
		glfwTerminate();
		return 0;
	}
	// using glfwWindowHint to configure GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// The following line is for Mac OS X to apply the configuration, or nothing works
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// create a window
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Window~", NULL, NULL);
	if (window == NULL) {
		cout << "Failed to create window!" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cout << "Failed to load GLAD!" << endl;
		return -1;
	}

	// tell OpenGL the size of the window for renderring
	// set offset and dimension
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// register callback function
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	// enable depth test
	glEnable(GL_DEPTH_TEST);
	// glDisable(GL_DEPTH_TEST);

	// ****************************************** data ************************************************
	// vertices data for "Color Edit"
	// define the triangle's vertices' position data 
	float A[2] = { -1.0f, -1.0f };
	float B[2] = { 1.0f, -1.0f };
	float C[2] = { 0.0f, 1.0f };

	// for "Bresenhem Triangle"
	// vertices data for "BresenHem Triangle"
	float v1[2] = { -0.5f, -0.5f };
	float v2[2] = { 0.5f, -0.5f };
	float v3[2] = { 0.0f, 0.5f };
	// for "Bresenhem Circle"
	// center for "Bresenmen Circle"
	float center[] = {0.0f, 0.0f};
	// radius for circle
	float radius = 0.5f;
	// edge length for square as a point
	float edge = 0.1f;

	// camera
	camera.aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

	// cubes
	Cube cubes[100];
	int size = 0;
	Cube* currentObject = new Cube();

	// plane
	Plane plane({ 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f, 0.0f}, {10.0f, 10.0f, 1.0f});


	// color of light
	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	// color of object
	glm::vec3 objectColor = { 1.0f, 0.5f, 0.31f };
	// lights
	Light sourceLight(glm::vec3(0.0f, 0.0f, 0.0f), lightColor, POINT_LIGHT);
	Light paralLight(glm::vec3(0.0f, 4.0f, 0.0f), lightColor, PARALELL_LIGHT);

	// shaders
	// for cube
	Shader cube(vss_cube, fss_cube);
	// for 2D object
	Shader graph2D(vertexShaderSource, fragmentShaderSource);
	Shader texture(vss_cube, fss_cube);
	Shader phong(phong_vertex_shader, phong_fragment_shader);
	Shader gouraud(gouraud_vertex_shader, gouraud_fragment_shader);
	Shader blinn(blinn_vertex_shader, blinn_fragment_shader);
	Shader depth(depth_vertex, depth_fragment);
	Shader currentShader = blinn;

	// textures
	// get texture manager's instance
	TextureManager* textureManager = TextureManager::getInstance();
	// load container for simple cube scene
	GLuint container = textureManager->load("Resources/Materials/Textures/container.jpg");
	cube.setInt("myTexture", container);
	
	// load diffuse texture and specular texture for light model scene
	GLuint floorTexture = textureManager->load("Resources/Materials/Textures/wall.jpg");
	GLuint diffuseTexture = textureManager->load("Resources/Materials/Textures/container2.jpg");
	GLuint specularTexture = textureManager->load("Resources/Materials/Textures/container2_specular.jpg");

	// Configure depth map FBO
	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// - Create depth texture
	GLuint depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// pass textures
	gouraud.use();
	gouraud.setInt("material.diffuse", 0);
	gouraud.setInt("material.specular", 1);
	gouraud.setInt("shadowMap", 2);
	gouraud.setFloat("light.constant", 1.0f);
	gouraud.setFloat("light.linear", 0.09f);
	gouraud.setFloat("light.quadratic", 0.032f);

	phong.use();
	phong.setInt("material.diffuse", 0);
	phong.setInt("material.specular", 1);
	phong.setInt("shadowMap", 2);
	phong.setFloat("light.constant", 1.0f);
	phong.setFloat("light.linear", 0.09f);
	phong.setFloat("light.quadratic", 0.032f);

	blinn.use();
	blinn.setInt("material.diffuse", 0);
	blinn.setInt("material.specular", 1);
	blinn.setInt("shadowMap", 2);
	blinn.setFloat("light.constant", 1.0f);
	blinn.setFloat("light.linear", 0.09f);
	blinn.setFloat("light.quadratic", 0.032f);

	// render a shadow texture
	// cut off plane for light's perspective
	GLfloat near_plane = 1.0f, far_plane = 7.5f;

	// ******************************************* UI ***************************************************
	// setup dear gui context
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// setup dear gui classic
	ImGui::StyleColorsClassic();
	// setup platform/renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	// GLSL version
	ImGui_ImplOpenGL3_Init("#version 330");
	// main loop
	while (!glfwWindowShouldClose(window)) {

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		// receive input
		// keyboard input
		processInput(window);
		// cursor action
		processCursor(window);
		glfwPollEvents();
		// create imgui
		// CREATE IMGUI
		// start dear gui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Crescent–«ø’", &isActive, ImGuiWindowFlags_MenuBar);
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("tools")) {
				if (ImGui::MenuItem("Color Edit", "Ctrl+E")) {
					option = 0;
				}
				if (ImGui::MenuItem("Triangle tool", "Ctrl+T")) {
					option = 1;
				}
				if (ImGui::MenuItem("Circle tool", "Ctrl+C")) {
					option = 2;
				}
				if (ImGui::MenuItem("3D tool", "Ctrl+ALT+C")) {
					option = 3;
				}
				if (ImGui::MenuItem("Close", "Ctrl+W")) {
					isActive = false;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		switch (option) {
		case 0:
			// imgui for color editing
			{
				// bind a array of type of float with the color editor
				ImGui::ColorEdit3("color", picker_color);
			}
			drawTriangle(A, B, C, picker_color, graph2D.ID);
			break;
		case 1:
			// draw a triangle with three given vertices using Bresenhem
			// imgui for bresenhem triangle
			{
				ImGui::BeginGroup();
				ImGui::ColorPicker3("select color", draw_color);
				ImGui::SliderFloat2("V1", v1, -0.9f, 0.9f, "%.1f");
				ImGui::SliderFloat2("V2", v2, -0.9f, 0.9f, "%.1f");
				ImGui::SliderFloat2("V3", v3, -0.9f, 0.9f, "%.1f");
				ImGui::EndGroup();
			}
			// draw grid
			drawGrid(19, 19, grid_color, graph2D.ID);
			// from v1 to v2
			myLineTo(v1, v2, draw_color, graph2D.ID);
			myLineTo(v1, v3, draw_color, graph2D.ID);
			myLineTo(v2, v3, draw_color, graph2D.ID);
			break;
		case 2:
			// draw a circle with a given origin and radius using Bresenhem
			// imgui for bresenhem circle
			{
				ImGui::BeginGroup();
				ImGui::ColorPicker3("select color", draw_color);
				ImGui::SliderFloat("Radius", &radius, 0.0f, 1.0f);
				ImGui::EndGroup();
			}
			drawGrid(19, 19, grid_color, graph2D.ID);
			myCircleAt(center, radius, draw_color, graph2D.ID);
			break;
		case 3:
			// 3D builder
			// imgui for Transform of the cube
			{
				ImGui::BeginGroup();
				if (ImGui::BeginMenuBar()) {
					if (ImGui::BeginMenu("3D Object")) {
						if (ImGui::MenuItem("Cube", "")) {
							Cube newCube({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
							cubes[size] = newCube;
							size++;
						}
						if (ImGui::BeginMenu("Light")) {
							if (ImGui::MenuItem(sourceLight.status.c_str())) {
								if (sourceLight.visible) {
									sourceLight.visible = false;
									sourceLight.status = "Invisible";
								}
								else {
									sourceLight.visible = true;
									sourceLight.status = "Visible";
								}
							}
							if (ImGui::BeginMenu("Light Mode")) {
								if (ImGui::MenuItem("Point Light")) {

								}
								if (ImGui::MenuItem("Parallel light")) {

								}
								ImGui::EndMenu();
							}
							if (ImGui::BeginMenu("Shading Mode")) {
								if (ImGui::MenuItem("Phong mode")) {
									currentShader = phong;
								}
								if (ImGui::MenuItem("Gouraud mode")) {
									currentShader = gouraud;
								}
								if (ImGui::MenuItem("Blinn mode")) {
									currentShader = blinn;
								}
								ImGui::EndMenu();
							}
							ImGui::EndMenu();
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("objects")) {
						for (int i = 0; i < size; i++) {
							if (ImGui::MenuItem(cubes[i].name.c_str(), "")) {
								currentObject = &cubes[i];
							}
						}
						ImGui::EndMenu();
					}
					ImGui::EndMenuBar();
				}
				
				float position[3] = { currentObject->transform.position[0], currentObject->transform.position[1], currentObject->transform.position[2] };
				float rotation[3] = { currentObject->transform.rotation[0], currentObject->transform.rotation[1], currentObject->transform.rotation[2] };
				float scale[3] = { currentObject->transform.scale[0], currentObject->transform.scale[1], currentObject->transform.scale[2] };
				float lightColor[3] = { paralLight.lightColor[0], paralLight.lightColor[1], paralLight.lightColor[2] };
				float lightPosition[3] = { paralLight.transform.position[0], paralLight.transform.position[1], paralLight.transform.position[2] };
				float objCol[3] = { objectColor[0], objectColor[1], objectColor[2] };
				ImGui::LabelText("Transform", currentObject->name.c_str());
				ImGui::SliderFloat3("Position", position, -20.0f, 20.0f);
				ImGui::SliderFloat3("Rotation", rotation, 0.0f, 180.0f);
				ImGui::SliderFloat3("Scale", scale, 0.0f, 5.0f);
				ImGui::LabelText("", "Light");
				ImGui::SliderFloat3("Light Position", lightPosition, -10.0f, 10.0f);
				ImGui::SliderFloat3("Light Color", lightColor, 0.0f, 1.0f);
				ImGui::SliderFloat("Ambient Factor", &sourceLight.ambientFactor, 0.0f, 1.0f);
				ImGui::SliderFloat("Diffuse Factor", &sourceLight.diffuseFactor, 0.0f, 1.0f);
				ImGui::SliderFloat("Specular Factor", &sourceLight.specularFactor, 0.0f, 10.0f);
				ImGui::SliderFloat("Shininess", &sourceLight.shininess, 1.0f, 64.0f);
				// set transform
				currentObject->transform.position = { position[0], position[1], position[2] };
				currentObject->transform.rotation = { rotation[0], rotation[1], rotation[2] };
				currentObject->transform.scale = { scale[0], scale[1], scale[2] };
				paralLight.transform.position = { lightPosition[0], lightPosition[1], lightPosition[2] };
				paralLight.lightColor = { lightColor[0], lightColor[1], lightColor[2] };
				objectColor = { objCol[0], objCol[1], objCol[2] };
				ImGui::EndGroup();
			}


			// transformation matrix from world space to light's perspective space
			// projection from light's perspective
			glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
			// parallel light
			glm::mat4 lightView = glm::lookAt(paralLight.transform.position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 lightSpaceMatrix = lightProjection * lightView;
			// - now render scene from light's point of view
			depth.use();
			glUniformMatrix4fv(glGetUniformLocation(depth.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			// render plane with depth texture renderred above
			plane.render(depth);
			// render cubes
			for (int i = 0; i < size; i++) {
				cubes[i].render(depth);
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);


			// render cubes with depth texture renderred above
			glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			blinn.use();
			glUniformMatrix4fv(glGetUniformLocation(blinn.ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
			// bind diffuse texture

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, floorTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, floorTexture);
			glActiveTexture(GL_TEXTURE10);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			// render plane with depth texture renderred above
			plane.render(camera, paralLight, blinn);

			// bind specular texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, diffuseTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, specularTexture);
			// render cubes
			for (int i = 0; i < size; i++) {
				cubes[i].render(camera, paralLight, blinn);
			}
			break;
		default:
			break;
		}
		ImGui::End();
		// RENDER
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		// check events and swap buffer
		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);
	}
	// shut down ImGui
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
	// terminate
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

// @dev This is a callback every time user change the size of the window.
// Corresponding change will be attached to view port for OpenGL renderring.
// @param window	The window pointer we have created
// @param width		New width
// @param height	New height
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

// @dev This is a function to check if some keys are pressed. 
// @param window	The window pointer we have created
void processInput(GLFWwindow *window)
{
	// if escape press, we will not receive any input anymore
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
		return;
	}
	int currentKeyState = glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT);
	if (key_grave_state == GLFW_RELEASE && currentKeyState == GLFW_PRESS) {
		int cursor_mode = glfwGetInputMode(window, GLFW_CURSOR);
		switch (cursor_mode) {
		case GLFW_CURSOR_DISABLED:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		case GLFW_CURSOR_NORMAL:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPos(window, xPos, yPos);
			break;
		default:
			break;
		}
	}
	key_grave_state = currentKeyState;
	float speed = camera.speed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.translate(-camera.transform.x * speed);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.translate(camera.transform.x * speed);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.translate(camera.transform.z * speed);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.translate(-camera.transform.z * speed);
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		camera.translate(-camera.transform.y * speed);
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		camera.translate(camera.transform.y * speed);
	}
}

void processCursor(GLFWwindow* window) {
	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
		glfwGetCursorPos(window, &xPos, &yPos);
		float translationX = -(float)xPos * 180.0f / 100000.0f;
		float translationY = (float)yPos * 90.0f / 100000.0f;
		float x = sin(translationX) * cos(translationY);
		float y = sin(translationY);
		float z = cos(translationX) * cos(translationY);
		camera.transform.forward = { x, y, z };
	}
	
}

void error_callback(int error, const char* description) {
	cout << stderr << "Error: " << description << endl;
}

// @dev create a grid style for background
// @param rows Rows of the grid
// @param cols Columns of the grid
// @param color Color for the grid
// @param shaderProgram Shader program for the grid
void drawGrid(int rows, int cols, float* color, unsigned int& shaderProgram) {
	for (int x = -rows/2; x <= rows/2; x++) {
		float v1[2] = { (float)x / 10.0f, -1.0f };
		float v2[2] = { (float)x / 10.0f, 1.0f };
		drawLine(v1, v2, color, shaderProgram);
	}
	for (int y = -cols / 2; y <= cols / 2; y++) {
		float v1[2] = { -1.0f, (float)y / 10.0f };
		float v2[2] = { 1.0f, (float)y / 10.0f };
		drawLine(v1, v2, color, shaderProgram);
	}
}

// @author –«ø’
// @dev This is a function using Bresenham to draw a line from one end to another
// @param v1				One End of a line
// @param v2				Another End of a line
// @param shaderProgram		Id of shader program
void myLineTo(float* v1, float* v2, float* color, unsigned int& shaderProgram) {

	float xi = v1[0], yi = v1[1];
	float dx, dy;
	float xs = v1[0] - v2[0];
	if (v1[0] != v2[0]) {
		dx = xs < 0? 0.1f : -0.1f;
		dy = (v1[1] - v2[1]) / xs * dx;
	}
	else {
		dx = 0;
		dy = 0.1f;
	}
	for (; (v1[0] - xi) / xs <= 1.01f; xi += dx, yi += dy) {
		int y_ = floor(yi * 10);
		float pi = yi * 10 - y_;
		if (pi > 0.5) {
			// draw
			drawSquare2D(xi, (float)(y_ + 1) / 10.0f, 0.1f, color, shaderProgram);
		}
		else {
			// draw
			drawSquare2D(xi, (float)y_ / 10.0f, 0.1f, color, shaderProgram);
		}
	}
	drawLine(v1, v2, color, shaderProgram);
}

// @dev This function implemented a circle drawing method based on Bresenham Algorithm displaying with
// several discrete points 
// @param center The center of the circle
// @param radius The radius of the circle
// @param color Color for drawing
// @param shaderProgram Shader program defined for drawing previously
void myCircleAt(float* center, float radius, float* color, unsigned int& shaderProgram) {

	float xi = 0, yi = radius;
	float dx = 0.1f;
	for (;xi <= radius;xi += dx, yi = sqrt(radius * radius - xi * xi)) {

		int y_ = floor(yi * 10);
		float pi = yi * 10 - y_;
		if (pi > 0.5) {
			yi = (float)(y_ + 1) / 10.0f;
		}
		else {
			yi = (float)y_ / 10.0f;
		}
		// symmetry
		drawSquare2D(xi, yi, 0.1f, color, shaderProgram);
		drawSquare2D(xi, -yi, 0.1f, color, shaderProgram);
		drawSquare2D(-xi, yi, 0.1f, color, shaderProgram);
		drawSquare2D(-xi, -yi, 0.1f, color, shaderProgram);

	}
	drawCircle(center, radius, color, 100, shaderProgram);
}

// @dev This function will draw a square according to given parameters by using glDrawElement()
// @param x X coordinate of the square
// @param y Y coordinate of the square
// @param edgeLength Length of edge of the square
// @param color Color of the square
// @param shaderProgram The shader program for shading the square
void drawSquare2D(float x, float y, float edgeLength, float* color, unsigned int& shaderProgram) {

	float vertices_[] = {
				x - 0.01, y - 0.01, 0.0f, color[0], color[1], color[2],
				x - 0.01, y + 0.01, 0.0f, color[0], color[1], color[2],
				x + 0.01, y + 0.01, 0.0f, color[0], color[1], color[2],
				x + 0.01, y - 0.01, 0.0f, color[0], color[1], color[2]
	};

	unsigned int indices[] = {
		0, 1, 2,
		0, 2, 3
	};

	unsigned int EBO;
	glGenBuffers(1, &EBO);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	unsigned int VBO;
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_), vertices_, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	// DRAW
	// enable the shader program
	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	// deallocate
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}


// @dev This function will draw a line according to given parameters by using primitive GL_LINE
// @param v1 Start point of the line
// @param v2 End point of the line
// @param color Color for stroking line
// @param shaderProgram The shader program for shading
void drawLine(float* v1, float* v2, float* color, unsigned int& shaderProgram) {

	float line[] = {
		v1[0], v1[1], 0.0f, color[0], color[1], color[2],
		v2[0], v2[1], 0.0f, color[0], color[1], color[2]
	};

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	unsigned int VBO;
	glGenBuffers(1, &VBO);


	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	// unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	// DRAW
	// enable the shader program
	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

// @dev This function will draw a circle according to given parameters by using primitive GL LINE_LOOP
// @param center Center of the circle which is default to (0.0f, 0.0f)
// @param radius Radius of the circle
// @param color Color strokinig the circle
// @param shaderProgram The shader program for shading
void drawCircle(float* center, float radius, float* color, int count, unsigned int& shaderProgram) {

	// initialize
	float* rad = new float[6 * count];
	for (int i = 0; i < count; i++) {
		// x
		int offset = i * 6;
		rad[offset] = cos(2.0f * PI / (float)count * (float)i) * radius;
		rad[offset + 1] = sin(2.0f * PI / (float)count * (float)i) * radius;
		rad[offset + 2] = 0.0f;
		rad[offset + 3] = color[0];
		rad[offset + 4] = color[1];
		rad[offset + 5] = color[2];
	}
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	unsigned int VBO;
	glGenBuffers(1, &VBO);


	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * count, rad, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	// unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	// DRAW
	// enable the shader program
	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINE_LOOP, 0, count);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

// @dev This function will draw a triangle according to given parameters by using primitive GL_TRIANGLE
// @param v1 First vertex of triangle
// @param v2 Second vertex of triangle
// @param v3 Third vertex of triangle
// @param color Color of the triangle
// @param shaderProgram The shader program for shading
void drawTriangle(float* v1, float* v2, float* v3, float* color, unsigned int& shaderProgram) {

	float vertices_[] = {
		v1[0], v1[1], 0.0f, color[0], color[1], color[2],// left
			v2[0], v2[1], 0.0f, color[0], color[1], color[2],// right
			v3[0],  v3[1], 0.0f, color[0], color[1], color[2] // top
	};

	// Vertex array object is to store every following vertex attributes callings
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	// Vetex buffer object is, obviously, a kind of object. Thus it has it's unique id
	// to identify it
	// vertex buffer object's id 
	unsigned int VBO;
	// generate buffer
	glGenBuffers(1, &VBO);
	// There are several type of buffer object. For vertex buffer object, it's buffer type
	// is called GL_ARRAY_BUFFER. OpenGL allows us to bind different kind of buffer type at 
	// the same time
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindVertexArray(VAO);
	// Now any operation calling on GL_ARRAY_BUFFER will configure on the buffer being binded
	// currently

	// @dev	This function is used to copy user's defined data into specified buffer
	// @param target	The kind of buffer to send to
	// @param size		The size of our data to send
	// @param data		The real data we want to send to buffer
	// @param usage		The way we hope our graphics card to manage the given data, with following three kinds of forms: 
	//			GL_STATIC_DRAW	- With data hardly changed
	//			GL_DYNAMIC_DRAW	- With data changed a lot
	//			GL_STREAM_DRAW	- With data changed once a drawing
	// And here we don't change the position of vertices of the triangle in every renderring
	// so it's best for us to choose GL_STATIC_DRAW 
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_), vertices_, GL_STATIC_DRAW);

	// @dev tell OpenGL how to explain vertex data and configure on each vertices
	// @param index			
	// @param size			Define size of this type of vertex, which is 3 for vec3
	// @param type			Define data type
	// @param normalized	Define whether we hope to normalize data or not
	// @param stride		Define length of each group of data
	// @param pointer		Define offset which is of type (void*)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	// unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	// DRAW
	// enable the shader program
	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}