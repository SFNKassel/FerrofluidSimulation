#include "window.h"
#include "shader.h"
#include "maths.h"

#include "buffers/buffer.h"
#include "buffers/indexbuffer.h"
#include "buffers/vertexarray.h"

#include <GLFW\glfw3.h>

int main() {
	using namespace engine;
	using namespace graphics;
	using namespace input;
	using namespace maths;
	using namespace utils;

	Window window("Ferrofluid", 1280, 720);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	Shader* shader = new Shader("shaders/balls.vert.glsl", "shaders/balls.frag.glsl");

	float angle = 0.0f;
	float zoom = 1.0f;

	Mat4 projectionMatrix = Mat4::orthographic(-8, 8, -4.5, 4.5, -1, 1);
	shader->setUniformMat4("pr_matrix", projectionMatrix);

	Mat4 projMat = projectionMatrix;

	while (!window.closed()) {
		window.clear();

		if (Input::isKeyPressed(GLFW_KEY_D))
			angle += 1.0;
		if (Input::isKeyPressed(GLFW_KEY_A))
			angle -= 1.0;
		if (Input::isKeyPressed(GLFW_KEY_W))
			zoom -= 0.1;
		if (Input::isKeyPressed(GLFW_KEY_S))
			zoom += 0.1;

		projMat = projectionMatrix * Mat4::scale(Vec3(1, 1, zoom)) * Mat4::rotation(angle, Vec3(0, 1, 0));
		shader->setUniformMat4("pr_matrix", projMat);

		window.update();
	}

	delete shader;

	return 0;
}