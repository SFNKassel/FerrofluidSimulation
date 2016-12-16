#include "input.h"

namespace engine {
	namespace input {

	bool Input::m_keys[MAX_KEYS], Input::m_keysBefore[MAX_KEYS], Input::m_keysTyped[MAX_KEYS];
	bool Input::m_mouseButtons[MAX_BUTTONS], Input::m_mouseButtonsBefore[MAX_BUTTONS], Input::m_mouseButtonsClicked[MAX_BUTTONS];
	double Input::m_mouseX, Input::m_mouseY;

	void Input::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
		m_keys[key] = (action != GLFW_RELEASE);
	}

	void Input::buttonCallback(GLFWwindow *window, int button, int action, int mods) {
		m_mouseButtons[button] = (action != GLFW_RELEASE);
	}

	void Input::cursorCallback(GLFWwindow *window, double x, double y) {
		m_mouseX = x;
		m_mouseY = y;
	}

	void Input::updateButtons() {
		for (int i = 0; i < MAX_KEYS; i++)
			m_keysTyped[i] = m_keys[i] && !m_keysBefore[i];

		for (int i = 0; i < MAX_BUTTONS; i++)
			m_mouseButtonsClicked[i] = m_mouseButtons[i] && !m_mouseButtonsBefore[i];

		memcpy(m_keysBefore, m_keys, MAX_KEYS * sizeof(bool));
		memcpy(m_mouseButtonsBefore, m_mouseButtons, MAX_BUTTONS * sizeof(bool));
	}

	void Input::init() {
		memset(m_keys, false, MAX_KEYS * sizeof(bool));
		memset(m_keysBefore, false, MAX_KEYS * sizeof(bool));
		memset(m_keysTyped, false, MAX_KEYS * sizeof(bool));

		memset(m_mouseButtons, false, MAX_BUTTONS * sizeof(bool));
		memset(m_mouseButtonsBefore, false, MAX_BUTTONS * sizeof(bool));
		memset(m_mouseButtonsClicked, false, MAX_BUTTONS * sizeof(bool));
	}

	bool Input::isKeyPressed(unsigned int keycode) {
		return m_keys[keycode];
	}

	bool Input::isKeyTyped(unsigned int keycode) {
		return m_keysTyped[keycode];
	}

	bool Input::isMouseButtonPressed(unsigned int button) {
		return m_mouseButtons[button];
	}

	bool Input::isMouseButtonClicked(unsigned int button) {
		return m_mouseButtonsClicked[button];
	}

	void Input::getMousePosition(double &x, double &y) {
		x = m_mouseX;
		y = m_mouseY;
	}
}}