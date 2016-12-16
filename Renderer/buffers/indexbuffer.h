#pragma once

#include <GL/glew.h>

namespace engine {
namespace graphics {

	class IndexBuffer {
	private:
		GLuint m_bufferID;
		GLuint m_count;
	public:
		IndexBuffer(GLushort* data, GLsizei count);
		~IndexBuffer();

		inline void bind() const;
		inline void unbind() const;

		void update(GLushort* data, GLsizei count) const;

		inline GLuint getCount() const { return m_count; };
	};

}}