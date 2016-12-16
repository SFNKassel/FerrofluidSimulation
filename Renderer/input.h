#pragma once

#include "window.h"

namespace engine {
namespace graphics {
	class Window;
}

namespace input {

#define MAX_KEYS 512
#define MAX_BUTTONS 32


class Input
	{
		friend class engine::graphics::Window;
	private:
		static bool m_keys[MAX_KEYS];
		static bool m_keysBefore[MAX_KEYS];
		static bool m_keysTyped[MAX_KEYS];
		static bool m_mouseButtons[MAX_BUTTONS];
		static bool m_mouseButtonsBefore[MAX_BUTTONS];
		static bool m_mouseButtonsClicked[MAX_BUTTONS];
		static double m_mouseX, m_mouseY;
		static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
		static void buttonCallback(GLFWwindow *window, int button, int action, int mods);
		static void cursorCallback(GLFWwindow *window, double x, double y);
		static void updateButtons();

		static void init();
	public:
		static bool isKeyPressed(unsigned int keycode);
		static bool isKeyTyped(unsigned int keycode);
		static bool isMouseButtonPressed(unsigned int keycode);
		static bool isMouseButtonClicked(unsigned int keycode);
		static void getMousePosition(double &x, double &y);
	};
}}