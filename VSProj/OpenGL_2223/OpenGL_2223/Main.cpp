#include <iostream>
#include <fstream>

//included glad before glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


//forward decs
void processInput(GLFWwindow* window);
int init(GLFWwindow*& window);
void createShaders();
void createProgram(GLuint& programID, const char* vertex, const char* fragment);
void createGeometry(GLuint& vao, GLuint& EBO, int& size, int& numIndices);
GLuint loadTexture(const char* path, int comp = 0, GLint wrapTypeS = GL_CLAMP_TO_EDGE, GLint wrapTypeT = GL_CLAMP_TO_EDGE);
GLuint loadCubemap(std::vector<string> fileNames, int comp = 0);
void renderSkyBox();
void renderStarBox();
void renderTerrain();
void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);
void renderPlanet();
void renderMoon(glm::mat4 parentPosition);
void createFrameBuffer(int width, int height, unsigned int& frameBufferID, unsigned int& colorBufferID, unsigned int& depthBufferID);
void renderToBuffer(unsigned int frameBufferTo, unsigned int colorBufferFrom, unsigned int shader);
void renderQuad();

unsigned int GeneratePlane(const char* heightmap, unsigned char*& data, GLenum format, int comp, float hScale, float xzScale, unsigned int& indexCount, unsigned int& heightmapID);

//window callbacks
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

bool keys[1024];

//util forward
void loadFile(const char* filename, char*& output);

//program IDs
GLuint simpleProgram, skyProgram, terrainProgram, modelProgram, starProgram, planetProgram, moonProgram, blitProgram, chrabbProgram, gamcorProgram, blurVerticalProgram, blurHorizontalProgram, bloomProgram;

const int WIDTH = 1920, HEIGHT = 1080;

//world data
glm::vec3 lightDirection = glm::normalize(glm::vec3(1.0f, 0, 0));
glm::vec3 cameraPosition = glm::vec3(150.0f, 0, 150.0f);
GLuint skyboxVAO;
GLuint skyboxEBO;
int skyboxSize;
int skyboxIndexCount;
glm::mat4 view;
glm::mat4 projection;

float lastX, lastY;
bool firstMouse = true;
float camYaw, camPitch;
glm::quat camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));

//terrain data
GLuint terrainVAO, terrainIndexCount, heightmapID, heightNormalID;
unsigned char* heightmapTexture;

GLuint dirt, sand, grass, rock, snow, cubeMap, day, night, clouds, moon;

Model* backpack, * sphere;
Model* arm;
Model* head;
Model* skull;
Model* alien;
unsigned int vertImg, fragImg;

int main()
{
	GLFWwindow* window;
	int res = init(window);
	if (res != 0) return res;

	createShaders();
	createGeometry(skyboxVAO, skyboxEBO, skyboxSize, skyboxIndexCount);

	terrainVAO = GeneratePlane("resources/textures/heightmap.png", heightmapTexture, GL_RGBA, 4, 250.0f, 5.0f, terrainIndexCount, heightmapID);
	heightNormalID = loadTexture("resources/textures/NormalMap.png");

	GLuint boxTex = loadTexture("resources/textures/container2.png");
	GLuint boxNormal = loadTexture("resources/textures/container2normal.png");

	dirt = loadTexture("resources/textures/dirt.jpg");
	sand = loadTexture("resources/textures/sand.jpg");
	grass = loadTexture("resources/textures/grass.png", 4);
	rock = loadTexture("resources/textures/rock.jpg");
	snow = loadTexture("resources/textures/snow.jpg");

	day = loadTexture("resources/textures/day.jpg");
	night = loadTexture("resources/textures/night.jpg");
	clouds = loadTexture("resources/textures/clouds.jpg", 0, GL_REPEAT, GL_CLAMP_TO_EDGE);
	moon = loadTexture("resources/textures/2k_moon.jpg");

	std::vector<string> fileNames =
	{
		"resources/textures/space-cubemap/right.png",
		"resources/textures/space-cubemap/left.png",
		"resources/textures/space-cubemap/top.png",
		"resources/textures/space-cubemap/bottom.png",
		"resources/textures/space-cubemap/front.png",
		"resources/textures/space-cubemap/back.png"
	};

	cubeMap = loadCubemap(fileNames);

	//arm = new Model("resources/Models/arm/arm.obj");
	//head = new Model("resources/Models/head/head.obj");
	//skull = new Model("resources/Models/skull/skull.obj");
	alien = new Model("resources/Models/alien/alien.obj");

	stbi_set_flip_vertically_on_load(true);
	sphere = new Model("resources/Models/uv_sphere.obj");

	//backpack = new Model("resources/Models/backpack/backpack.obj");

	//tell opengl to create viewport
	glViewport(0, 0, WIDTH, HEIGHT);

	unsigned int frameBuf1, frameBuf2;
	unsigned int colorBuf1, colorBuf2;
	unsigned int depthBuf1, depthBuf2;

	createFrameBuffer(WIDTH, HEIGHT, frameBuf1, colorBuf1, depthBuf1);
	createFrameBuffer(WIDTH, HEIGHT, frameBuf2, colorBuf2, depthBuf2);

	//matrices
	view = glm::lookAt(cameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	projection = glm::perspective(glm::radians(60.0f), WIDTH / (float)HEIGHT, 1.0f, 10000.0f);

	//OPENGL SETTINGS
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	//rendering loop
	while (!glfwWindowShouldClose(window))
	{
		//input
		processInput(window);

		glBindFramebuffer(GL_FRAMEBUFFER, frameBuf1);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// rendering
		//renderSkyBox();
		renderStarBox();
		renderPlanet();
		//renderTerrain();

		float t = glfwGetTime();

		//renderModel(backpack, glm::vec3(1000,100,1000), glm::vec3(0,t,0), glm::vec3(100,100,100));
		//renderModel(arm, glm::vec3(100, 100, 1000), glm::vec3(0, t, 0), glm::vec3(5, 5, 5));
		//renderModel(head, glm::vec3(1000, 100, 100), glm::vec3(-90, t, 0), glm::vec3(100, 100, 100));
		//renderModel(skull, glm::vec3(1300, 100, 1300), glm::vec3(0, t, 0), glm::vec3(100, 100, 100));
		renderModel(alien, glm::vec3(100, 100, 100), glm::vec3(0, t, 0), glm::vec3(10, 10, 10));
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//POST PROCESSING
		//renderToBuffer(frameBuf2, colorBuf1, chrabbProgram);
		renderToBuffer(frameBuf2, colorBuf1, gamcorProgram);
		renderToBuffer(frameBuf1, colorBuf2, blurVerticalProgram);
		//renderToBuffer(frameBuf2, colorBuf1, blurHorizontalProgram);
		//renderToBuffer(frameBuf2, colorBuf1, bloomProgram);
		renderToBuffer(0, colorBuf2, blitProgram);

		//swap&poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void renderSkyBox()
{
	//opengl setup
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(skyProgram);

	glm::mat4 world = glm::mat4(1.0f);
	world = glm::translate(world, cameraPosition);
	world = glm::scale(world, glm::vec3(10, 10, 10));

	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3fv(glGetUniformLocation(skyProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(skyProgram, "cameraPosition0"), 1, glm::value_ptr(cameraPosition));

	//rendering
	glBindVertexArray(skyboxVAO);
	glDrawElements(GL_TRIANGLES, skyboxIndexCount, GL_UNSIGNED_INT, 0);

	//opengl closing
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH);
}

void renderStarBox()
{
	//opengl setup
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(starProgram);

	glm::mat4 world = glm::mat4(1.0f);
	world = glm::translate(world, cameraPosition);
	world = glm::scale(world, glm::vec3(10, 10, 10));

	glUniformMatrix4fv(glGetUniformLocation(starProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(starProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(starProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3fv(glGetUniformLocation(starProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(starProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	//rendering
	glBindVertexArray(skyboxVAO);
	glDrawElements(GL_TRIANGLES, skyboxIndexCount, GL_UNSIGNED_INT, 0);

	//opengl closing
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH);
}

void renderTerrain()
{
	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(terrainProgram);

	glm::mat4 world = glm::mat4(1.0f);

	glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//float t = glfwGetTime();
	//lightDirection = glm::normalize(glm::vec3(glm::sin(t), -0.5f, glm::cos(t)));
	glUniform3fv(glGetUniformLocation(terrainProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(terrainProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmapID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, heightNormalID);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, dirt);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, sand);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, grass);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, rock);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, snow);


	//rendering
	glBindVertexArray(terrainVAO);
	glDrawElements(GL_TRIANGLES, terrainIndexCount, GL_UNSIGNED_INT, 0);

}

unsigned int GeneratePlane(const char* heightmap, unsigned char*& data, GLenum format, int comp, float hScale, float xzScale, unsigned int& indexCount, unsigned int& heightmapID) {
	int width, height, channels;
	data = nullptr;
	if (heightmap != nullptr) {
		data = stbi_load(heightmap, &width, &height, &channels, comp);
		if (data) {
			glGenTextures(1, &heightmapID);
			glBindTexture(GL_TEXTURE_2D, heightmapID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	int stride = 8;
	float* vertices = new float[(width * height) * stride];
	unsigned int* indices = new unsigned int[(width - 1) * (height - 1) * 6];

	int index = 0;
	for (int i = 0; i < (width * height); i++) {
		// TODO: calculate x/z values
		int x = i % width;
		int z = i / width;

		float texHeight = (float)data[i * comp];

		// TODO: set position
		vertices[index++] = x * xzScale;
		vertices[index++] = (texHeight / 255.0f) * hScale;
		vertices[index++] = z * xzScale;

		// TODO: set normal
		vertices[index++] = 0;
		vertices[index++] = 1;
		vertices[index++] = 0;

		// TODO: set uv
		vertices[index++] = x / (float)width;
		vertices[index++] = z / (float)height;
	}

	// OPTIONAL TODO: Calculate normal
	// TODO: Set normal

	index = 0;
	for (int i = 0; i < (width - 1) * (height - 1); i++) {
		// TODO: calculate x/z values
		int x = i % (width - 1);
		int z = i / (width - 1);

		int vertex = z * width + x;

		indices[index++] = vertex;
		indices[index++] = vertex + width;
		indices[index++] = vertex + width + 1;

		indices[index++] = vertex;
		indices[index++] = vertex + width + 1;
		indices[index++] = vertex + 1;
	}

	unsigned int vertSize = (width * height) * stride * sizeof(float);
	indexCount = ((width - 1) * (height - 1) * 6);

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	// vertex information!
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, 0);
	glEnableVertexAttribArray(0);
	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);
	// uv
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * 6));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	delete[] vertices;
	delete[] indices;

	//stbi_image_free(data);

	return VAO;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	bool camChanged = false;

	if (keys[GLFW_KEY_W])
	{
		cameraPosition += camQuat * glm::vec3(0, 0, 1);
		camChanged = true;
	}
	if (keys[GLFW_KEY_S])
	{
		cameraPosition += camQuat * glm::vec3(0, 0, -1);
		camChanged = true;
	}
	if (keys[GLFW_KEY_A])
	{
		cameraPosition += camQuat * glm::vec3(1, 0, 0);
		camChanged = true;
	}
	if (keys[GLFW_KEY_D])
	{
		cameraPosition += camQuat * glm::vec3(-1, 0, 0);
		camChanged = true;
	}

	if (camChanged)
	{
		glm::vec3 camForward = camQuat * glm::vec3(0, 0, 1);
		glm::vec3 camUp = camQuat * glm::vec3(0, 1, 0);
		view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);
	}
}

int init(GLFWwindow*& window)
{
	//glfw init
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//glfw window creation
	window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL_2223", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	//register callbacks
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);

	//load glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	float x = (float)xpos;
	float y = (float)ypos;

	if (firstMouse)
	{
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	float dx = x - lastX;
	float dy = y - lastY;
	lastX = x;
	lastY = y;

	camYaw -= dx;
	camPitch = glm::clamp(camPitch + dy, -90.0f, 90.0f);
	if (camYaw > 180.0f) camYaw -= 360.0f;
	if (camYaw < -180.0f) camYaw += 360.0f;

	camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));

	glm::vec3 camForward = camQuat * glm::vec3(0, 0, 1);
	glm::vec3 camUp = camQuat * glm::vec3(0, 1, 0);
	view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		//store key is pressed
		keys[key] = true;
	}
	else if (action == GLFW_RELEASE)
	{
		//store key is not pressed
		keys[key] = false;
	}
}

void createGeometry(GLuint& vao, GLuint& EBO, int& size, int& numIndices)
{
	float vertices[] = {
		// positions            //colors            // tex coords   // normals          //tangents      //bitangents
		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 0.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,

		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,      -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,      -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,

		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,      0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,
		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,      0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,

		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,

		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   1.f, 1.f,       -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,

		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,
		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,

		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f
	};

	int indices[] =
	{
		// DOWN
		0, 1, 2,   // first triangle
		0, 2, 3,    // second triangle
		// BACK
		14, 6, 7,   // first triangle
		14, 7, 15,    // second triangle
		// RIGHT
		20, 4, 5,   // first triangle
		20, 5, 21,    // second triangle
		// LEFT
		16, 8, 9,   // first triangle
		16, 9, 17,    // second triangle
		// FRONT
		18, 10, 11,   // first triangle
		18, 11, 19,    // second triangle
		// UP
		22, 12, 13,   // first triangle
		22, 13, 23,    // second triangle
	};

	int stride = (3 + 3 + 2 + 3 + 3 + 3) * sizeof(float);
	size = sizeof(vertices) / stride;
	numIndices = sizeof(indices) / sizeof(int);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(11 * sizeof(float)));
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, stride, (void*)(14 * sizeof(float)));
	glEnableVertexAttribArray(5);

}

void createShaders()
{
	createProgram(simpleProgram, "resources/Shaders/simpleVertex.shader", "resources/Shaders/simpleFragment.shader");

	//set texture channels
	glUseProgram(simpleProgram);
	glUniform1i(glGetUniformLocation(simpleProgram, "mainTex"), 0);
	glUniform1i(glGetUniformLocation(simpleProgram, "normalTex"), 1);

	createProgram(skyProgram, "resources/Shaders/skyVertex.shader", "resources/Shaders/skyFragment.shader");
	createProgram(terrainProgram, "resources/Shaders/terrainVertex.shader", "resources/Shaders/terrainFragment.shader");

	glUseProgram(terrainProgram);
	glUniform1i(glGetUniformLocation(terrainProgram, "mainTex"), 0);
	glUniform1i(glGetUniformLocation(terrainProgram, "normalTex"), 1);

	glUniform1i(glGetUniformLocation(terrainProgram, "dirt"), 2);
	glUniform1i(glGetUniformLocation(terrainProgram, "sand"), 3);
	glUniform1i(glGetUniformLocation(terrainProgram, "grass"), 4);
	glUniform1i(glGetUniformLocation(terrainProgram, "rock"), 5);
	glUniform1i(glGetUniformLocation(terrainProgram, "snow"), 6);

	createProgram(modelProgram, "resources/Shaders/model.vs", "resources/Shaders/model.fs");
	glUseProgram(modelProgram);

	/*
	uniform sampler2D texture_diffuse1;
	uniform sampler2D texture_specular1;
	uniform sampler2D texture_normal1;
	uniform sampler2D texture_roughness1;
	uniform sampler2D texture_ao1;
	*/
	glUniform1i(glGetUniformLocation(modelProgram, "texture_diffuse1"), 0);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_specular1"), 1);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_normal1"), 2);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_roughness1"), 3);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_ao1"), 4);

	createProgram(starProgram, "resources/Shaders/skyVertex.shader", "resources/Shaders/starBox.fs");

	createProgram(planetProgram, "resources/Shaders/model.vs", "resources/Shaders/planet.fs");
	glUseProgram(planetProgram);
	glUniform1i(glGetUniformLocation(planetProgram, "day"), 0);
	glUniform1i(glGetUniformLocation(planetProgram, "night"), 1);
	glUniform1i(glGetUniformLocation(planetProgram, "clouds"), 2);

	createProgram(moonProgram, "resources/Shaders/model.vs", "resources/Shaders/moon.fs");


	///post processing///
	createProgram(blitProgram, "resources/Shaders/vertImage.vs", "resources/Shaders/fragImage.fs");
	createProgram(chrabbProgram, "resources/Shaders/vertImage.vs", "resources/Shaders/fragImagechrabb.fs");
	createProgram(gamcorProgram, "resources/Shaders/vertImage.vs", "resources/Shaders/fragImageGammaCor.fs");
	createProgram(blurVerticalProgram, "resources/Shaders/vertImage.vs", "resources/Shaders/fragImageblur.fs");
	//createProgram(blurHorizontalProgram, "resources/Shaders/vertImage.vs", "resources/Shaders/fragImageblur.fs");
	//glUniform1i(glGetUniformLocation(blurHorizontalProgram, "horizontal"), true);
	//createProgram(bloomProgram, "resources/Shaders/vertImage.vs", "resources/Shaders/fragImagebloom.fs");
}

void createProgram(GLuint& programID, const char* vertex, const char* fragment)
{
	//create a gl program with a vertex & fragment shader
	char* vertexSrc;
	char* fragmentSrc;
	loadFile(vertex, vertexSrc);
	loadFile(fragment, fragmentSrc);

	GLuint vertexShaderId, fragmentShaderID;

	vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderId, 1, &vertexSrc, nullptr);
	glCompileShader(vertexShaderId);

	int success;
	char infoLog[512];
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, nullptr, infoLog);
		std::cout << "ERROR COMPILING VERTEX SHADER\n" << infoLog << std::endl;
	}

	fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, &fragmentSrc, nullptr);
	glCompileShader(fragmentShaderID);

	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderID, 512, nullptr, infoLog);
		std::cout << "ERROR COMPILING FRAGMENT SHADER\n" << infoLog << std::endl;
	}

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderId);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programID, 512, nullptr, infoLog);
		std::cout << "ERROR LINKING PROGRAM\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderID);

	delete vertexSrc;
	delete fragmentSrc;
}

void loadFile(const char* filename, char*& output)
{
	std::ifstream file(filename, std::ios::binary); //open file

	if (file.is_open())	//if the file was correctly opened
	{
		file.seekg(0, file.end); // get length of file
		int length = file.tellg();
		file.seekg(0, file.beg); // seek to start of file

		output = new char[length + 1]; //allocate memory for the char pointer
		file.read(output, length); // read data as a block
		output[length] = '\0'; // add null teminator to end of char pointer

		file.close(); //close file
	}
	else
	{
		output = NULL; //if the file failed to open, set the char pointer to null
	}
}

GLuint loadTexture(const char* path, int comp, GLint wrapTypeS, GLint wrapTypeT)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapTypeS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapTypeT);

	int width, height, numChannels;
	unsigned char* data = stbi_load(path, &width, &height, &numChannels, comp);
	if (data)
	{
		if (comp != 0) numChannels = comp;
		if (numChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else if (numChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Error loading texture: " << path << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

GLuint loadCubemap(std::vector<string> fileNames, int comp)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, numChannels;
	for (int i = 0; i < fileNames.size(); i++)
	{
		unsigned char* data = stbi_load(fileNames[i].c_str(), &width, &height, &numChannels, comp);
		if (data)
		{
			if (comp != 0) numChannels = comp;
			if (numChannels == 3)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			else if (numChannels == 4)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			std::cout << "Error loading texture: " << fileNames[i].c_str() << std::endl;
		}
		stbi_image_free(data);
	}

	//tex settings
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return textureID;
}

void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
{
	//glEnable(GL_BLEND);
	//alpha blend
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//additive blend
	//glBlendFunc(GL_ONE, GL_ONE);
	//soft additive blend
	//glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
	//multiply blend
	//glBlendFunc(GL_DST_COLOR, GL_ZERO);
	//double multiply blend
	//glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(modelProgram);

	glm::mat4 world = glm::mat4(1.0f);
	world = glm::translate(world, pos);
	world = world * glm::toMat4(glm::quat(rot));
	world = glm::scale(world, scale);

	glUniformMatrix4fv(glGetUniformLocation(modelProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(modelProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(modelProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3fv(glGetUniformLocation(modelProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(modelProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	model->Draw(modelProgram);
	glDisable(GL_BLEND);
}

void renderPlanet()
{
	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(planetProgram);

	glm::mat4 world = glm::mat4(1.0f);
	world = glm::translate(world, glm::vec3(0, 0, 0));
	world = glm::scale(world, glm::vec3(100, 100, 100));
	world = glm::rotate(world, glm::radians(23.0f), glm::vec3(1, 0, 0));
	world = glm::rotate(world, glm::radians((float)glfwGetTime()), glm::vec3(0, 1, 0));

	glUniformMatrix4fv(glGetUniformLocation(planetProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(planetProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(planetProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3fv(glGetUniformLocation(planetProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(planetProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	glUniform1f(glGetUniformLocation(planetProgram, "time"), (float)glfwGetTime());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, day);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, night);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, clouds);

	sphere->Draw(planetProgram);
	glDisable(GL_BLEND);

	glm::mat4 parentPosition = glm::mat4(1.0f);
	parentPosition = glm::translate(parentPosition, glm::vec3(0, 0, 0));

	renderMoon(parentPosition);
}

void renderMoon(glm::mat4 parentPosition)
{
	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(moonProgram);

	glm::mat4 world = glm::mat4(1.0f);
	world = parentPosition;
	world = glm::rotate(world, glm::radians(5.0f), glm::vec3(1, 0, 0));
	world = glm::rotate(world, glm::radians((float)glfwGetTime() * (48 / 60.0f)), glm::vec3(0, 1, 0));
	world = glm::translate(world, glm::vec3(0, 0, 6033));
	world = glm::scale(world, glm::vec3(25, 25, 25));

	glUniformMatrix4fv(glGetUniformLocation(moonProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(moonProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(moonProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3fv(glGetUniformLocation(moonProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(moonProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, moon);

	sphere->Draw(moonProgram);
}

void createFrameBuffer(int width, int height, unsigned int& frameBufferID, unsigned int& colorBufferID, unsigned int& depthBufferID)
{
	//gen frame buffer
	glGenFramebuffers(1, &frameBufferID);

	//gen color buffer
	glGenTextures(1, &colorBufferID);
	glBindTexture(GL_TEXTURE_2D, colorBufferID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	//gen depth buffer
	glGenRenderbuffers(1, &depthBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

	//attach buffers
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferID, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ) {std::cout << "Framebuffer not complete!" << std::endl;}

	//unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderToBuffer(unsigned int frameBufferTo, unsigned int colorBufferFrom, unsigned int shader)
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferTo);

	glUseProgram(shader);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBufferFrom);

	//render something
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		//setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}