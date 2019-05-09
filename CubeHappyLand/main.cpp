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

#include "Camera.h"
#include "Light.h"
#include "Cube.h"

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
// set shader program we have defined
void setShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource, unsigned int& shaderProgram);
// dividebackground into grid
void drawGrid(int rows, int cols, float* color, unsigned int& shaderProgram);
// draw line from v1 to v2 using Bresenhem
void myLineTo(float* v1, float* v2, float* color, unsigned int& shaderProgram);
// draw circle at center with radius using Bresenhem
void myCircleAt(float* center, float radius, float* color, unsigned int& shaderProgram);
// build a 3D cube
void myCubeBuild(const float* vertices, float edgeLength, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, const char* texture_path, unsigned int shaderProgram);
// draw square with primitive GL_TRIANLE
void drawSquare2D(float xi_1, float yi_1, float edgeLength, float* color, unsigned int& shaderProgram);
// draw line with primitive GL_LINE
void drawLine(float* v1, float* v2, float* color, unsigned int& shaderProgram);
// draw circle with primitive GL_LINE_LOOP
void drawCircle(float* center, float radius, float* color, int count, unsigned int& shaderProgram);
// draw triangle with primitive GL_TRIANLE
void drawTriangle(float* v1, float* v2, float* v3, float* color, unsigned int& shaderProgram);

// VERTEX SHADER
// source code for vertex shader
const char* vertexShaderSource = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aCol;\n"
"out vec3 myColor;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(aPos, 1.0f);\n"
"	myColor = aCol;\n"
"}\n";
// source code for cube's vertex shader
const char* vss_cube = "#version 330 core\n"
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
// FRAGMENT SHADER
// cource code for fragment shader
const char* fragmentShaderSource = "#version 330 core\n"
// @keyword out Declare a output variable
"out vec4 FragColor;\n"
"in vec3 myColor;\n"
"void main()\n"
"{\n"
"	FragColor = vec4(myColor, 1.0f);\n"
"}\n";
// source code for cube fragment shader
const char* fss_cube = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 TexCoord;\n"
"uniform sampler2D myTexture;\n"
"void main()\n"
"{\n"
"	FragColor = texture(myTexture, TexCoord);\n"
"}\n";

const char* phong_vertex_shader = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aNormal;\n"
"out vec3 FragPos;\n"
"out vec3 Normal;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"	FragPos = vec3(model * vec4(aPos, 1.0f));\n"
"	gl_Position = projection * view * vec4(FragPos, 1.0f);\n"
"	Normal = mat3(transpose(inverse(model))) * aNormal;\n"
"}\n";

const char* phong_fragment_shader = "#version 330 core\n"
"in vec3 FragPos;\n"
"in vec3 Normal;\n"
"out vec4 FragColor;\n"
"uniform vec3 objectColor;\n"
"uniform vec3 lightColor;\n"
"uniform vec3 viewPos;\n"
"uniform vec3 lightPos;\n"
"uniform float ambientFactor;\n"
"uniform float diffuseFactor;\n"
"uniform float specularFactor;\n"
"uniform float shininess;\n"
"void main()\n"
"{\n"
"	vec3 ambient = ambientFactor * lightColor;\n"
"	vec3 normal = normalize(Normal);\n"
"	vec3 lightDirection = normalize(lightPos - FragPos);\n"
"	float diffuseStrength = max(dot(normal, lightDirection), 0.0f);\n"
"	vec3 diffuse = diffuseStrength * lightColor * diffuseFactor;\n"
"	vec3 viewDirection = normalize(viewPos - FragPos);\n"
"	vec3 reflectDirection = normalize(reflect(-lightDirection, normal));\n"
"	float specularStrength = max(pow(dot(viewDirection, reflectDirection), shininess), 0.0f);\n"
"	vec3 specular = specularStrength * lightColor * specularFactor;\n"
"	vec3 result =  (ambient + diffuse + specular) * objectColor ;\n"
"	FragColor = vec4(result, 1.0f);\n"
"}\n";

const char* gouraud_vertex_shader = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aNormal;\n"
"out vec3 color;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"uniform vec3 viewPos;\n"
"uniform vec3 lightPos;\n"
"uniform vec3 objectColor;\n"
"uniform vec3 lightColor;\n"
"uniform float ambientFactor;\n"
"uniform float diffuseFactor;\n"
"uniform float specularFactor;\n"
"uniform float shininess;\n"
"void main() {\n"
"	vec3 FragPos = vec3(model * vec4(aPos, 1.0f));\n"
"	gl_Position = projection * view * vec4(FragPos, 1.0f);\n"
"	vec3 Normal = mat3(transpose(inverse(model))) * aNormal;\n"
"	vec3 ambient = ambientFactor * lightColor;\n"
"	vec3 normal = normalize(Normal);\n"
"	vec3 lightDirection = normalize(lightPos - FragPos);\n"
"	float diffuseStrength = max(dot(normal, lightDirection), 0.0f);\n"
"	vec3 diffuse = diffuseStrength * lightColor * diffuseFactor;\n"
"	vec3 viewDirection = normalize(viewPos - FragPos);\n"
"	vec3 reflectDirection = normalize(reflect(-lightDirection, normal));\n"
"	float specularStrength = max(pow(dot(viewDirection, reflectDirection), shininess), 0.0f);\n"
"	vec3 specular = specularStrength * lightColor * specularFactor;\n"
"	color =  (ambient + diffuse + specular) * objectColor ;\n"
"}";
const char* gouraud_fragment_shader = "#version 330 core\n"
"in vec3 color;\n"
"out vec4 FragColor;\n"
"void main() {\n"
"	FragColor = vec4(color, 1.0f);\n"
"}";
// mouse position
double xPos = 0;
double yPos = 0;
// imgui's status
bool isActive = true;
// user's option
int option = DEFAULT_OPTION;
// picker's color
float* picker_color = new float[3];
// grid color
float grid_color[3] = { 1.0f, 1.0f, 1.0f };
// line and circles' color
float draw_color[3] = { 0.0f, 0.0f, 1.0f };

// camera
Camera camera(45.0f, 1.0f, 100.0f);
int key_grave_state = GLFW_RELEASE;

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

	// for "Build My Cube"
	float camera_position[3] = { 0.0f, 0.0f, -3.0f };
	camera.aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
	Cube cubes[100];

	int size = 0;
	Cube* currentObject = new Cube();

	// light shaders
	Light sourceLight(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	// color of object
	glm::vec3 objectColor = { 1.0f, 0.5f, 0.31f };

	// set shader program
	unsigned int shaderProgram, shaderProgram_cube;
	// for cube
	setShaderProgram(vss_cube, fss_cube, shaderProgram_cube);
	// for 2D object
	setShaderProgram(vertexShaderSource, fragmentShaderSource, shaderProgram);
	Shader texture(vss_cube, fss_cube);
	Shader phong(phong_vertex_shader, phong_fragment_shader);
	Shader gouraud(gouraud_vertex_shader, gouraud_fragment_shader);
	Shader currentShader = phong;

	// enable depth test
	glEnable(GL_DEPTH_TEST);
	// glDisable(GL_DEPTH_TEST);

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
		// receive input
		// keyboard input
		processInput(window);
		// cursor action
		processCursor(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwPollEvents();
		// create imgui
		// CREATE IMGUI
		// start dear gui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("CrescentÐÇ¿Õ", &isActive, ImGuiWindowFlags_MenuBar);
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
			drawTriangle(A, B, C, picker_color, shaderProgram);
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
			drawGrid(19, 19, grid_color, shaderProgram);
			// from v1 to v2
			myLineTo(v1, v2, draw_color, shaderProgram);
			myLineTo(v1, v3, draw_color, shaderProgram);
			myLineTo(v2, v3, draw_color, shaderProgram);
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
			drawGrid(19, 19, grid_color, shaderProgram);
			myCircleAt(center, radius, draw_color, shaderProgram);
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
							if (ImGui::MenuItem("Phong mode")) {
								currentShader = phong;
							}
							if (ImGui::MenuItem("Gouraud mode")) {
								currentShader = gouraud;
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
				float lightColor[3] = { sourceLight.lightColor[0], sourceLight.lightColor[1], sourceLight.lightColor[2] };
				float lightPosition[3] = { sourceLight.transform.position[0], sourceLight.transform.position[1], sourceLight.transform.position[2] };
				float objCol[3] = { objectColor[0], objectColor[1], objectColor[2] };
				ImGui::LabelText("Transform", currentObject->name.c_str());
				ImGui::SliderFloat3("Position", position, -10.0f, 10.0f);
				ImGui::SliderFloat3("Rotation", rotation, 0.0f, 180.0f);
				ImGui::SliderFloat3("Scale", scale, 0.0f, 5.0f);
				ImGui::LabelText("", "Light");
				ImGui::SliderFloat3("Light Position", lightPosition, -10.0f, 10.0f);
				ImGui::SliderFloat3("Light Color", lightColor, 0.0f, 1.0f);
				ImGui::SliderFloat3("Object Color", objCol, 0.0f, 1.0f);
				ImGui::SliderFloat("Ambient Factor", &sourceLight.ambientFactor, 0.0f, 1.0f);
				ImGui::SliderFloat("Diffuse Factor", &sourceLight.diffuseFactor, 0.0f, 1.0f);
				ImGui::SliderFloat("Specular Factor", &sourceLight.specularFactor, 0.0f, 10.0f);
				ImGui::SliderFloat("Shininess", &sourceLight.shininess, 1.0f, 64.0f);
				// set transform
				currentObject->transform.position = { position[0], position[1], position[2] };
				currentObject->transform.rotation = { rotation[0], rotation[1], rotation[2] };
				currentObject->transform.scale = { scale[0], scale[1], scale[2] };
				sourceLight.transform.position = { lightPosition[0], lightPosition[1], lightPosition[2] };
				sourceLight.lightColor = { lightColor[0], lightColor[1], lightColor[2] };
				objectColor = { objCol[0], objCol[1], objCol[2] };
				ImGui::EndGroup();
			}
			// render bulb
			if (sourceLight.visible) {
				sourceLight.render(camera);
			}
			// render cubes
			for (int i = 0; i < size; i++) {
				cubes[i].render(camera, sourceLight, currentShader, objectColor);
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

// @dev In OpenGL nowadays, we should at least define a vertex shader and a fragments shader if we want to do some
// renderring. We are going to use GLSL(OpenGL Shading Language) to program our shaders. We can use them after
// they are having compiled.
// @param vertexShaderSource Source code string of vertex shader program
// @param fragmentShaderSource Source code string of fragment shader program
// @param shaderProgram Id of the shader program
void setShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource, unsigned int& shaderProgram) {
	
	// shader object's id
	unsigned int vertexShader;
	// create a vertex shader and return its id, which we will use it as a reference to the very shader
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	// @dev set shader's source code
	// @param shader	The shader'id
	// @param count		The count of number of source code string
	// @param string	The source code
	// @param length	The length of the whole source code(?)
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// check whether it is compiled successfully
	// flag
	int success;
	// compile information log
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		// get compile error message
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// fragment shader's id
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check whether it is compiled successfully
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		// get compile error message
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
	}
	// create shader program
	shaderProgram = glCreateProgram();
	// TESTS AND BLENDING
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		cout << "ERROR::SHDAER::PROGRAM::LINK_FAILED\n" << infoLog << endl;
	}

	// delete shaders cause we won't use it anymore
	glUseProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
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

// @author ÐÇ¿Õ
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