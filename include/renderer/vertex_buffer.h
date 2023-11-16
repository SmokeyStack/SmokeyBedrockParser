#pragma once

#include <cstdint>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace smokey_bedrock_parser {
	class VertexBuffer {
	public:
		VertexBuffer(const void* data, uint32_t size);
		~VertexBuffer();

		void Bind();
		void Unbind();
	private:
		uint32_t renderer_id;
	};
}