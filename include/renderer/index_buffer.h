#pragma once

#include <cstdint>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace smokey_bedrock_parser {
	class IndexBuffer {
	public:
		IndexBuffer(const uint32_t* data, uint32_t count);
		~IndexBuffer();

		void Bind();
		void Unbind();
	private:
		uint32_t renderer_id;
		uint32_t count;
	};
}