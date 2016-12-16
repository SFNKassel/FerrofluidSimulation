#include "shader.h"

namespace engine {
namespace graphics {

	Shader::Shader(const char *vertexPath, const char *fragmentPath) : m_vertexPath(vertexPath), m_fragmentPath(fragmentPath){
		m_Shader = load();
	}

	Shader::~Shader() {
		glDeleteProgram(m_Shader);
	}

	void Shader::setUniform1i(const GLchar* name, const GLint& value){
		glUniform1i(getUniformLocation(name), value);
	}

	void Shader::setUniform1iv(const GLchar* name, GLint* value, GLsizei count) {
		glUniform1iv(getUniformLocation(name), count, value);
	}

	void Shader::setUniform1f(const GLchar* name, const GLfloat& value) {
		glUniform1f(getUniformLocation(name), value);
	}

	void Shader::setUniform1fv(const GLchar* name, GLfloat* value, GLsizei count) {
		glUniform1fv(getUniformLocation(name), count, value);
	}

	void Shader::setUniform2f(const GLchar* name, const maths::Vec2& vector) {
		glUniform2f(getUniformLocation(name), vector.x, vector.y);
	}

	void Shader::setUniform3f(const GLchar* name, const maths::Vec3& vector) {
		glUniform3f(getUniformLocation(name), vector.x, vector.y, vector.z);
	}

	void Shader::setUniform4f(const GLchar* name, const maths::Vec4& vector) {
		glUniform4f(getUniformLocation(name), vector.x, vector.y, vector.z, vector.w);
	}

	void Shader::setUniformMat4(const GLchar* name, const maths::Mat4& matrix) {
		glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, matrix.elements);
	}

	GLint Shader::getUniformLocation(const GLchar *name) {
		return glGetUniformLocation(m_Shader, name);
	}

	GLuint Shader::load() {
		GLuint program = glCreateProgram();
		GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);

		std::string vertSourceStr = utils::FileUtils::read_file(m_vertexPath);
		std::string fragSourceStr = utils::FileUtils::read_file(m_fragmentPath);

		const char* vertSource = vertSourceStr.c_str();
		const char* fragSource = fragSourceStr.c_str();

		glShaderSource(vertex, 1, &vertSource, NULL);
		glCompileShader(vertex);

		GLint result;
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &result);

		if (result == GL_FALSE) {
			GLint length;
			glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &length);
			char* error = new char[length];
			glGetShaderInfoLog(vertex, length, &length, error);
			std::cout << "Failed to Compile Vertex Shader" << std::endl << error << std::endl;
			glDeleteShader(vertex);
			return 0;
		}

		glShaderSource(fragment, 1, &fragSource, NULL);
		glCompileShader(fragment);

		glGetShaderiv(fragment, GL_COMPILE_STATUS, &result);

		if (result == GL_FALSE) {
			GLint length;
			glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &length);
			char* error = new char[length];
			glGetShaderInfoLog(fragment, length, &length, error);
			std::cout << "Failed to Compile Fragment Shader" << std::endl << error << std::endl;
			glDeleteShader(fragment);
			return 0;
		}

		glAttachShader(program, vertex);
		glAttachShader(program, fragment);

		glLinkProgram(program);
		glValidateProgram(program);

		glDeleteShader(vertex);
		glDeleteShader(fragment);

		return program;
	}

	void Shader::enable() const {
		glUseProgram(m_Shader);
	}

	void Shader::disable() const {
		glUseProgram(NULL);
	}

}}