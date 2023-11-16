#include "renderer/index_buffer.h"

namespace smokey_bedrock_parser {
    IndexBuffer::IndexBuffer(const uint32_t* data, uint32_t count) : count(count) {
        glGenBuffers(1, &renderer_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(data), data, GL_STATIC_DRAW);
    }

    IndexBuffer::~IndexBuffer() {
        glDeleteBuffers(1, &renderer_id);
    }

    void IndexBuffer::Bind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id);

    }

    void IndexBuffer::Unbind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    }
} // namespace smokey_bedrock_parser