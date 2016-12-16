#pragma once

#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <iostream>

#include "input.h"

#define MAX_KEYS 512
#define MAX_BUTTONS 32

namespace engine {
namespace graphics {

class Window {
	private:
		const char *m_title;
		int m_width, m_height;
		GLFWwindow *m_window;
		bool m_closed;

		bool init();
		static void windowResize(GLFWwindow *window, int width, int height);
	public:
		Window(const char *name, int width, int height);
		~Window();

		inline int getWidth() { return m_width; }
		inline int getHeight() { return m_height; }

		void clear() const;
		void update();
		bool closed() const;
	};
}}