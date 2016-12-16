#include "indexbuffer.h"

namespace engine {
namespace graphics {

	IndexBuffer::IndexBuffer(GLushort* data, GLsizei count) : m_count(count) {
		glGenBuffers(1, &m_bufferID);
		bind();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLushort), data, GL_DYNAMIC_DRAW);
		unbind();
	}

	IndexBuffer::~IndexBuffer() {
		glDeleteBuffers(1, &m_bufferID);
	}

	void IndexBuffer::bind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
	}

	void IndexBuffer::unbind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void IndexBuffer::update(GLushort *data, GLsizei count) const {
		bind();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLushort), data, GL_DYNAMIC_DRAW);
		unbind();
	}

}}