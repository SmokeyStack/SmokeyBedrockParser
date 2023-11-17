#include "renderer/vertex_buffer.h"

#include "logger.h"

namespace smokey_bedrock_parser {
	VertexBuffer::VertexBuffer(const void* data, uint32_t size) {
		glGenBuffers(1, &renderer_id);
		glBindBuffer(GL_ARRAY_BUFFER, renderer_id);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}

	VertexBuffer::~VertexBuffer() {
		glDeleteBuffers(1, &renderer_id);
	}

	void VertexBuffer::Bind() {
		glBindBuffer(GL_ARRAY_BUFFER, renderer_id);

	}

	void VertexBuffer::Unbind() {
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	}
} // namespace smokey_bedrock_parser