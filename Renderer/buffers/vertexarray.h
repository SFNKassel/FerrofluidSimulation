#pragma once

#include <GL/glew.h>
#include <vector>

#include "buffer.h"

namespace engine { namespace graphics {

	class VertexArray {
	private:
		GLuint m_arrayID;
		std::vector<Buffer*> m_buffers;
	public:
		VertexArray();
		~VertexArray();
		void addBuffer(Buffer* buffer, GLuint index);

		inline void bind() const;
		inline void unbind() const;
	};

}}