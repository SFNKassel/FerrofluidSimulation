// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <geometry/icosphere.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <cpptoml.h>
#include <istream>

using namespace std;

// Shader sources
const GLchar * vertexSource = "#version 330 core\n"
							  "in vec3 position;"
							  "in vec4 center;"
							  "in vec3 color;"
							  "in vec2 texcoord;"
							  "out vec3 Color;"
							  "out vec2 Texcoord;"
							  "uniform mat4 model;"
							  "uniform mat4 view;"
							  "uniform mat4 proj;"
							  "uniform vec3 origin;"
							  "uniform float scale;"
							  "void main()"
							  "{"
							  "    Color = color;"
							  "    Texcoord = texcoord;"
							  "    gl_Position = proj * view * model * "
							  "vec4((position * center.w + origin +"
							  "center.xyz) * scale, 1.0);"
							  "}";

const GLchar * fragmentSource = "#version 330 core\n"
								"in float vertexID;"
								"in vec3 Color;"
								"in vec2 Texcoord;"
								"out vec4 outColor;"
								"void main()"
								"{"
								"    outColor = vec4(Color, 1.0);"
								"}";
/* "        outColor = vec4(1.0, 0.0, 0.0, 1.0);" */

static void error_callback(int error, const char * description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow * window, int key, int scancode, int action,
                         int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void prepareBuffers(int vao, int vbo, int ebo, IcoSphere & s) {
	std::vector<GLfloat> vertices(s.points().size() * 8, 0.0);
	std::vector<GLuint>  elems(s.faces().size() * 3, 0);
	int                  i = 0;

	for(const auto & vertex : s.points()) {
		// std::cout << vertex << std::endl;
		vertices[i++] = vertex.x;
		vertices[i++] = vertex.y;
		vertices[i++] = vertex.z;

		// Color
		vertices[i++] = std::abs(vertex.y) / vertex.norm();
		vertices[i++] = std::abs(vertex.z) / vertex.norm();
		vertices[i++] = std::abs(vertex.x) / vertex.norm();

		// vertices[i++] = 1;
		// vertices[i++] = 0;
		// vertices[i++] = 0;

		// unused
		vertices[i++] = 0;
		vertices[i++] = 0;
	}

	i = 0;
	for(const auto & face : s.faces()) {
		for(const auto & elem : face.vertices()) { elems[i++] = elem; }
	}

	glBindVertexArray(vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elems.size() * sizeof(GLuint),
				 elems.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat),
				 vertices.data(), GL_STATIC_DRAW);

	// glBindVertexArray(0);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	// glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// state
bool running   = true;
bool leftDown  = false;
bool rightDown = false;
s64 originX = 0, originY = 0, totalX = 0, totalY = 0, width = 800, height = 600;
float     ratio    = float(width) / float(height);
float     distance = 0.4f;
glm::mat4 model;
glm::mat4 proj;
glm::mat4 view;
GLint     uniView;
GLint     uniModel;
GLint     uniProj;
GLint     uniScale;
GLint     uniOrigin;
glm::vec4 xAxis(0.0f, 0.0f, 1.0f, 0.0f);
glm::vec4 yAxis(1.0f, 0.0f, 0.0f, 0.0f);
glm::vec3 origin(0.0f, 0.0f, 0.0f);

static void mouse_button_callback(GLFWwindow * window, int button, int action,
								  int mods) {
	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		leftDown = true;
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		originX = x;
		originY = y;
	} else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		leftDown = false;
	} else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		rightDown = true;
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		originX = x;
		originY = y;
	} else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		rightDown = false;
	}
}

static void mouse_move_callback(GLFWwindow * window, double currentX,
								double currentY) {
	if(leftDown) {
		s64 deltaX = (originX - currentX) * 0.5;
		s64 deltaY = (originY - currentY) * 0.5;
		totalX += deltaX;
		totalY += deltaY;

		model =
			glm::rotate(model, glm::radians(-float(deltaX)),
						glm::vec3(glm::normalize(glm::inverse(model) * xAxis)));

		model =
			glm::rotate(model, glm::radians(float(deltaY)),
						glm::vec3(glm::normalize(glm::inverse(model) * yAxis)));

		// model = tmpModel2 * tmpModel;
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

		originX = currentX;
		originY = currentY;
	} else if(rightDown) {
		f64 deltaX = float(originX - currentX) / float(width);
		f64 deltaY = float(originY - currentY) / float(height);

		/*
		 * view = glm::translate(view, glm::vec3(deltaX, 0.0, deltaY));
		 * glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
		 */
		auto tmp = glm::vec4(origin, 1.0) +
				   glm::inverse(model) * glm::vec4(deltaX, 0.0, deltaY, 1.0);

		origin = glm::vec3(tmp.x, tmp.y, tmp.z);
		/* origin = origin + model * glm::vec3(deltaX, 0.0, deltaY, 0.0); */
		glUniform3fv(uniOrigin, 1, glm::value_ptr(origin));

		originX = currentX;
		originY = currentY;
	}
}

static void reshape(GLFWwindow * window, int w, int h) {
	width  = w;
	height = h;
	glViewport(0, 0, w, h);

	ratio = float(w) / float(h);

	proj = glm::perspective(glm::radians(45.0f), ratio, 1.0f, 10.0f);

	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
}
static void scroll_callback(GLFWwindow * window, double xoffset,
							double yoffset) {
	if(yoffset > 0) {
		::distance *= 0.95;
	} else {
		::distance *= 1.05;
	}

	glUniform1f(uniScale, ::distance);
}

void update_center(ifstream & file, long particles, double radius,
				   GLuint center_vbo) {
	std::vector<v3>      center(particles);
	std::vector<GLfloat> centers(particles * 4);

	file.read((char *)center.data(), sizeof(v3) * center.size());

	for(s64 i = 0; i < particles; i++) {
		centers[4 * i]     = center[i].comp[0];
		centers[4 * i + 1] = center[i].comp[1];
		centers[4 * i + 2] = center[i].comp[2];
		centers[4 * i + 3] = radius * 200; // TODO(robin): random magic constant
	}

	glBindBuffer(GL_ARRAY_BUFFER, center_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * centers.size(),
				 centers.data(), GL_STREAM_DRAW);
}

int main(int argc, char ** argv) {
	long     particles;
	long     timesteps;
	long     current_timestep;
	double   radius;
	ifstream sim_file;
	sim_file.exceptions(ifstream::failbit | ifstream::badbit);

	if(argc != 2) {
		cerr << "Exactly one argument required!" << endl;
		exit(-1);
	} else {
		try {
			auto table = cpptoml::parse_file(argv[1]);
			particles =
				*table->get_qualified_as<long>("SimulationData.Particles");
			timesteps =
				*table->get_qualified_as<long>("SimulationData.Timesteps");
			radius = *table->get_qualified_as<double>(
				"SimulationParameters.RadiusParticle");
			cout << "timesteps: " << timesteps << endl;
			cout << "particles: " << particles << endl;
			auto tmp          = string(argv[1]);
			auto sim_filename = tmp.substr(0, tmp.size() - 9);
			cout << "sim_filename: " << sim_filename << endl;
			sim_file = ifstream(sim_filename, ios::binary);
		} catch(const cpptoml::parse_exception & e) {
			cerr << "Failed to parse config file: " << e.what() << endl;
			exit(-1);
		} catch(std::ifstream::failure & failure) {
			cerr << "Error during reading metadata occured: " << failure.what();
			exit(-1);
		}
	}

	GLFWwindow * window;
	glfwSetErrorCallback(error_callback);

	if(!glfwInit()) { exit(EXIT_FAILURE); }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(width, height, "Ferrofluid", NULL, NULL);
	if(!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);
	if(!gladLoadGL()) {
		printf("GLAD: could not load GL\n");
		exit(-1);
	}

	printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);
	if(GLVersion.major < 4) {
		printf("Your system doesn't support OpenGL >= 2!\n");
		return -1;
	}

	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
		   glGetString(GL_SHADING_LANGUAGE_VERSION));

	// icosahedron
	flr::f64             t = (1.0 + sqrt(5.0)) / 2.0;
	std::vector<flr::v3> verts{{-1, t, 0}, {1, t, 0}, {-1, -t, 0}, {1, -t, 0},
							   {0, -1, t}, {0, 1, t}, {0, -1, -t}, {0, 1, -t},
							   {t, 0, -1}, {t, 0, 1}, {-t, 0, -1}, {-t, 0, 1}};
	std::vector<flr::Face> fcs{
		{{0, 11, 5}}, {{0, 5, 1}},  {{0, 1, 7}},   {{0, 7, 10}}, {{0, 10, 11}},
		{{1, 5, 9}},  {{5, 11, 4}}, {{11, 10, 2}}, {{10, 7, 6}}, {{7, 1, 8}},
		{{3, 9, 4}},  {{3, 4, 2}},  {{3, 2, 6}},   {{3, 6, 8}},  {{3, 8, 9}},
		{{4, 9, 5}},  {{2, 4, 11}}, {{6, 2, 10}},  {{8, 6, 7}},  {{9, 8, 1}}};

	flr::Mesh      m(verts, fcs);
	flr::IcoSphere s(m, 0.4, 4);

	GLuint vao;
	glGenVertexArrays(1, &vao);

	GLuint ebo;
	glGenBuffers(1, &ebo);

	GLuint vbo;
	glGenBuffers(1, &vbo);

	prepareBuffers(vao, vbo, ebo, s);

	GLuint center_vbo;
	glGenBuffers(1, &center_vbo);
	/*
	 * glBindBuffer(GL_ARRAY_BUFFER, center_vbo);
	 * std::vector<GLfloat> center = {3, 0, 0, 1, -3, 0, 0, 2};
	 * /\* std::vector<GLfloat> center = {0, 0, 0, 1}; *\/
	 * glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * center.size(),
	 * 			 center.data(), GL_STREAM_DRAW);
	 */

	// Create and compile the vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	GLint isCompiled = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &errorLog[0]);

		std::cerr << &errorLog[0] << std::endl;

		glDeleteShader(vertexShader);

		std::exit(EXIT_FAILURE);
	}

	// Create and compile the fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &errorLog[0]);

		std::cerr << &errorLog[0] << std::endl;

		glDeleteShader(fragmentShader);

		std::exit(EXIT_FAILURE);
	}

	// Link the vertex and fragment shader into a shader program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	// Specify the layout of the vertex data
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
						  0);

	GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
	glEnableVertexAttribArray(colAttrib);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
						  reinterpret_cast<void *>((3 * sizeof(GLfloat))));

	GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
						  reinterpret_cast<void *>(6 * sizeof(GLfloat)));

	GLint centerAttrib = glGetAttribLocation(shaderProgram, "center");
	glEnableVertexAttribArray(centerAttrib);
	glBindBuffer(GL_ARRAY_BUFFER, center_vbo);
	glVertexAttribPointer(centerAttrib, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);

	glVertexAttribDivisor(posAttrib, 0);
	glVertexAttribDivisor(colAttrib, 0);
	glVertexAttribDivisor(texAttrib, 0);
	glVertexAttribDivisor(centerAttrib, 1);

	uniModel = glGetUniformLocation(shaderProgram, "model");

	// Set up projection
	view = glm::lookAt(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
					   glm::vec3(0.0f, 0.0f, 1.0f));
	uniView = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

	proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 10.0f);
	uniProj = glGetUniformLocation(shaderProgram, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	glEnable(GL_DEPTH_TEST);

	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

	uniScale = glGetUniformLocation(shaderProgram, "scale");
	glUniform1f(uniScale, ::distance);

	uniOrigin = glGetUniformLocation(shaderProgram, "origin");
	glUniform3fv(uniOrigin, 1, glm::value_ptr(origin));

	glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetWindowSizeCallback(window, reshape);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	while(!glfwWindowShouldClose(window)) {
		update_center(sim_file, particles, radius, center_vbo);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* glDrawElements(GL_TRIANGLES, s.faces().size() * 3, GL_UNSIGNED_INT,
		 * 0); */
		glDrawElementsInstanced(GL_TRIANGLES, s.faces().size() * 3,
								GL_UNSIGNED_INT, 0, particles);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
