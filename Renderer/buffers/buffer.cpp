#include "buffer.h"

namespace engine {
namespace graphics {
	
	Buffer::Buffer(GLfloat* data, GLsizei count, GLuint componentCount) : m_componentCount(componentCount) {
		glGenBuffers(1, &m_bufferID);
		bind();
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(GLfloat), data, GL_STATIC_DRAW);
		unbind();
	}

	Buffer::~Buffer() {
		glDeleteBuffers(1, &m_bufferID);
	}

	void Buffer::bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
	}
	
	void Buffer::unbind() const {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	
}}