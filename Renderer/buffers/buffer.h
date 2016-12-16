#pragma once

#include <GL/glew.h>

namespace engine {
namespace graphics {

	class Buffer {
	private:
		GLuint m_bufferID;
		GLuint m_componentCount;
	public:
		Buffer(GLfloat* data, GLsizei count, GLuint componentCount);
		~Buffer();

		inline void bind() const;
		inline void unbind() const;

		inline GLuint getComponentCount() const { return m_componentCount; };
	};

}}