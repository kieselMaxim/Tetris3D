#ifndef BLOCK_H
#define BLOCK_H


#include "Shader.h"
#include "Cube.h"


class Block {
public:

	Shader shader;
	Cube blockObject;
	glm::vec4 color;

	Block(){}

	Block(Shader sh, Cube cu,glm::vec4 co) {
		shader = sh;
		blockObject = cu;
		color = co;
	}

	//Zeichent Block an bestimmter Stelle mit bestimmter Rotation
	void draw(glm::mat4 model, glm::mat4 transform) {
		shader.setInt("effect", 0);
		shader.setVec4("color", color);
		shader.setMat4("model", model);
		shader.setMat4("transform", transform);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	





};
#endif