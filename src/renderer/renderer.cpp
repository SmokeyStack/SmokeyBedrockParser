#include "renderer/renderer.h"

#include <cstdint>
#include <glad/glad.h>

const int max_blocks = 256;
const int max_indices = 6 * max_blocks;
const int max_vertices = 4 * max_blocks;

struct Vertex {
	glm::vec2 position;
	glm::vec4 color;
	int grid_step;
};

namespace smokey_bedrock_parser {
	struct RendererData {
		GLuint VAO = 0;
		GLuint VBO = 0;
		GLuint IBO = 0;

		uint32_t index_counter = 0;

		Vertex* quad_buffer = nullptr;
		Vertex* quad_buffer_ptr = nullptr;

		Renderer::Stats stats;
	};

	static RendererData data;

	void Renderer::Init() {
		data.quad_buffer = new Vertex[max_vertices];

		glCreateVertexArrays(1, &data.VAO);
		glBindVertexArray(data.VAO);
		glGenBuffers(1, &data.VBO);
		glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
		glBufferData(GL_ARRAY_BUFFER, max_vertices * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, grid_step));
		glEnableVertexAttribArray(2);

		uint32_t indices[max_indices];
		int offset = 0;

		for (int a = 0; a < max_indices; a += 6) {
			indices[a] = offset;
			indices[a + 1] = offset + 1;
			indices[a + 2] = offset + 2;

			indices[a + 3] = offset + 2;
			indices[a + 4] = offset + 3;
			indices[a + 5] = offset;
			offset += 4;
		}

		glGenBuffers(1, &data.IBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	}

	void Renderer::Shutdown() {
		glDeleteVertexArrays(1, &data.VAO);
		glDeleteBuffers(1, &data.VBO);
		glDeleteBuffers(1, &data.IBO);

		delete[] data.quad_buffer;
	}

	void Renderer::BeginScene() {
		data.quad_buffer_ptr = data.quad_buffer;
	}

	void Renderer::EndScene() {
		GLsizeiptr size = (uint8_t*)data.quad_buffer_ptr - (uint8_t*)data.quad_buffer;
		glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data.quad_buffer);
	}

	void Renderer::Flush() {
		glBindVertexArray(data.VAO);
		glDrawElements(GL_TRIANGLES, data.index_counter, GL_UNSIGNED_INT, nullptr);
		data.stats.draw_calls++;

		data.index_counter = 0;
	}

	void Renderer::DrawQuad(const glm::vec2 position, const int step, glm::vec3 color) {
		if (data.index_counter > max_indices) {
			EndScene();
			Flush();
			BeginScene();
		}

		data.quad_buffer_ptr->position = position;
		data.quad_buffer_ptr->color = glm::vec4(color, 1.0f);
		data.quad_buffer_ptr->grid_step = step;
		data.quad_buffer_ptr++;

		data.quad_buffer_ptr->position = glm::vec2(position.x + step, position.y);
		data.quad_buffer_ptr->color = glm::vec4(color, 1.0f);
		data.quad_buffer_ptr->grid_step = step;
		data.quad_buffer_ptr++;

		data.quad_buffer_ptr->position = glm::vec2(position.x + step, position.y - step);
		data.quad_buffer_ptr->color = glm::vec4(color, 1.0f);
		data.quad_buffer_ptr->grid_step = step;
		data.quad_buffer_ptr++;

		data.quad_buffer_ptr->position = glm::vec2(position.x, position.y - step);
		data.quad_buffer_ptr->color = glm::vec4(color, 1.0f);
		data.quad_buffer_ptr->grid_step = step;
		data.quad_buffer_ptr++;

		data.index_counter += 6;
		data.stats.quad_count++;
	}

	const Renderer::Stats& Renderer::GetStats() {
		return data.stats;
	}

	void Renderer::ResetStats() {
		memset(&data.stats, 0, sizeof(Stats));
	}
}	// namespace smokey_bedrock_parser