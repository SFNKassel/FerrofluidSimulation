#pragma once

#include <GL/glew.h>
#include <iostream>
#include "fileutils.h"
#include "maths.h"

namespace engine {
namespace graphics {

	class Shader {
	private:
		GLuint m_Shader;
		const char *m_vertexPath, *m_fragmentPath;

		GLuint load();
		GLint getUniformLocation(const GLchar* name);
	public:
		Shader(const char *vertexPath, const char *fragmentPath);
		~Shader();

		void setUniform1i(const GLchar* name, const GLint& value);
		void setUniform1iv(const GLchar* name, GLint* value, GLsizei count);
		void setUniform1f(const GLchar* name, const GLfloat& value);
		void setUniform1fv(const GLchar* name, GLfloat* value, GLsizei count);
		void setUniform2f(const GLchar* name, const maths::Vec2& vector);
		void setUniform3f(const GLchar* name, const maths::Vec3& vector);
		void setUniform4f(const GLchar* name, const maths::Vec4& vector);
		void setUniformMat4(const GLchar* name, const maths::Mat4& matrix);

		void enable() const;
		void disable() const;
	};

}}