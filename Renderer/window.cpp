#include "window.h"

namespace engine {
namespace graphics {

	Window::Window(const char *title, int width, int height) {
		m_title = title;
		m_width = width;
		m_height = height;
		if (!init())
			glfwTerminate();
		
	}

	Window::~Window() {
		glfwTerminate();
	}

	void Window::windowResize(GLFWwindow *window, int width, int height) {
		glViewport(0, 0, width, height);
		Window* win = (Window*)glfwGetWindowUserPointer(window);
		win->m_width = width;
		win->m_height = height;
	}

	bool Window::init() {
		if (!glfwInit()) {
			std::cout << "Failed to init GLFW" << std::endl;
			return false;
		}

		m_window = glfwCreateWindow(m_width, m_height, m_title, NULL, NULL);
		if (m_window == NULL) {
			std::cout << "Failed to create GLFW Window" << std::endl;
			return false;
		}

		input::Input::init();

		glfwSetFramebufferSizeCallback(m_window, Window::windowResize);
		glfwSetWindowUserPointer(m_window, this);
		glfwSetKeyCallback(m_window, input::Input::keyCallback);
		glfwSetMouseButtonCallback(m_window, input::Input::buttonCallback);
		glfwSetCursorPosCallback(m_window, input::Input::cursorCallback);
		glfwMakeContextCurrent(m_window);
		glfwSwapInterval(0);


		if (glewInit() != GLEW_OK)
		{
			std::cout << "Failed to Init GLEW" << std::endl;;
			return false;
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;

		return true;
	}

	bool Window::closed() const {
		return glfwWindowShouldClose(m_window) == GL_TRUE;
	}

	void Window::clear() const {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Window::update(){
		GLenum err = glGetError();

		if (err != GL_NO_ERROR)
			std::cout << "OpenGL Error: " << err << std::endl;

		glfwPollEvents();
		glfwSwapBuffers(m_window);

		input::Input::updateButtons();
	}
}}