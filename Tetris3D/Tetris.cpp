#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "camera.h"
#include "Cube.h"
#include "Gamesector.h"
#include "Tetromino.h"
#include "Quad.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
bool tetrominoIsStillMoving();
bool tetrominoIsStillRotating();
unsigned int bindUniformBuffer(Shader shader);
unsigned int loadTexture(char const* path);

// Fenstergröße
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// Kamera Variablen
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;	
float lastFrame = 0.0f;

//Variablen für die Bewegung der Blöcke
float m_speed = 0.0;
float r_speed = 0.0;
glm::mat4 oldPos = glm::mat4(1.0f);
float rotateDirection = 0.0f;
float moveDirection = 0.0f;
bool rotateBlock = false;
bool moveDown = false;

//Größe und Entfernung des Tetrisspielfeldes
int height = 22;
int width = 12;
float distance = 25.0f;

//post Processing Einstellung
int postProcessing = 1;

//Position der Lichtquelle
glm::vec3 lightPos(0.0f, 10.0f, -10.0f);


//Main Methode
int main()
{
	//Window creation
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tetris3D", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Mithilfe der Shader Klasse wird hier ein Shaderprogramm erstellt
	Shader shader("VertexShader.shader","FragmentSHader.shader");

	Cube cube = Cube();
	Quad quad = Quad(shader);
	cube.texture = loadTexture("boxTexture.jpg");

	Block gruenerBlock(shader, cube, glm::vec4(0.0, 1.0, 0.0, 1.0));
	Block grauerBlock(shader, cube,glm::vec4(0.5, 0.5, 0.5, 1.0));
	Block roterBlock(shader, cube,glm::vec4(1.0, 0.3, 0.3, 1.0));
	Block blauerBlock(shader, cube,glm::vec4(0.0, 0.0, 1.0, 1.0));

	std::vector <Block> blocks = { gruenerBlock, roterBlock, blauerBlock };

	//Spielfeld mit Rändern wird erstellt
	std::vector<std::vector<GameSector> > gamefield(width, std::vector<GameSector>(height));
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			//Worldposition für die Blöcke wird generiert und in dem gamefield vector gespeichert, /2.0f damit sich alles im zentrum befindet
			glm::vec3 pos(i - (width / 2.0f), j - (height / 2.0f), -distance);
			if (i == width-1 || j == height-1) {
				GameSector gs(pos, grauerBlock);
				gs.blocked = true;
				gamefield[i][j] = gs;
				continue;
			}
			if (i > 0 && j > 0) {
				GameSector gs(pos);
				gs.blocked = false;
				gamefield[i][j] = gs;
				continue;
			}
			GameSector gs(pos, grauerBlock);
			gs.blocked = true;
			gamefield[i][j] = gs;
		}
	}

	//Das bewegbare Objekt wird erstellt
	Tetromino t1(&gamefield,blocks);

	//Framebuffer
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	//textureColorbuffer für post post processing
	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	//Renderbuffer
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer error";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	unsigned int uniformShader = bindUniformBuffer(shader);

	//Shader wird aktiviert und die zu verwendende Texturen dem Shader übergeben
	shader.use();
	shader.setInt("texture1", 0);
	shader.setInt("screenTexture", 0);


	//render loop
	while (!glfwWindowShouldClose(window))
	{
		//Zeit die vergangen ist seit dem letzten Frame wird hier gespeichert
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Eingaben werden überprüft
		processInput(window);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST);

		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Uniforms die für die Kammera benögt werden, werden an den Shader übergeben
		glm::mat4 view = camera.GetViewMatrix();
		glBindBuffer(GL_UNIFORM_BUFFER, uniformShader);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		//Bindings für die Blöcke
		glBindVertexArray(cube.VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cube.texture);

		//Licht variablen an Shader übergeben
		shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		shader.setVec3("lightPos", lightPos);

		//Gamefield vektor wird mit einer schleife durchgegangen und jeder block darin wird auf dem Bildschirm gezeichnet
		for (unsigned int i = 0; i < width; i++)
		{
			for (unsigned int j = 0; j < height; j++)
			{
				glm::mat4 model = glm::mat4(1.0f);
				glm::mat4 transform = glm::mat4(1.0f);
				model = glm::translate(model, gamefield[i][j].WorldPosition);
				gamefield[i][j].block.draw(model,transform);
			}
		}

		//Wenn der vorherige Tetromino am Boden angekommen ist wird ein neuer erstellt
		if (t1.reachedBottom) {
			t1 = Tetromino(&gamefield, blocks);
		}

		//Tetromino wird gezeichnet
		t1.draw(&moveDirection, &rotateDirection, &moveDown);

		//Quad wird vor der Kammera gezeichnet
		quad.draw(&textureColorbuffer, postProcessing);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &cube.VAO);
	glDeleteVertexArrays(1, &quad.VAO);
	glDeleteBuffers(1, &cube.VBO);
	glDeleteBuffers(1, &quad.VBO);

	glfwTerminate();
	return 0;
}

//Methode um vielen Shaderprogrammen die Uniformdaten übergeben zu können. In diesem Fall die Projection und View Matrix
unsigned int bindUniformBuffer(Shader shader) {
	unsigned int uniformBlockIndex = glGetUniformBlockIndex(shader.ID, "Matrices");
	glUniformBlockBinding(shader.ID, uniformBlockIndex, 0);

	unsigned int uboMatrices;
	glGenBuffers(1, &uboMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));
	glm::mat4 projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return uboMatrices;
}

//Input wird überprüft
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//Kamera Steuerung
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	//Steurung der Blöcke nach links/rechts
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		if (tetrominoIsStillMoving() && tetrominoIsStillRotating()) {
			moveDown = false;
			moveDirection = 1.0f;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		if (tetrominoIsStillMoving() && tetrominoIsStillRotating()) {
			moveDown = false;
			moveDirection = -1.0f;
		}
	}
	
	//Steuerung um Blöcke nach unten zu bewegen
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (tetrominoIsStillMoving() && tetrominoIsStillRotating()) {
			moveDirection = -1.0f;
			moveDown = true;
		}
	}
	//Steurung um Blöcke zu rotieren
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		if (tetrominoIsStillRotating()) {
			rotateDirection = -1.0f;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		if (tetrominoIsStillRotating()) {
			rotateDirection = 1.0f;
		}
	}

	//Steuerung der post processing Effekte
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		postProcessing = 2;
	}
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
		postProcessing = 1;
	}	
}

bool tetrominoIsStillMoving() {
	return moveDirection == 0;
}

bool tetrominoIsStillRotating() {
	return rotateDirection == 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; 
	lastX = xpos;
	lastY = ypos;
	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

//Methode um Texturen zu laden und an unsigned in zu binden
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	return textureID;
}