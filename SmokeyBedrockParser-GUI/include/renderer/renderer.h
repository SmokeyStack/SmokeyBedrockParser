#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace smokey_bedrock_parser {
	class Renderer {
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene();
		static void EndScene();
		static void Flush();

		static void DrawQuad(const glm::vec2 position, const int step, glm::vec3 color);

		struct Stats {
			int draw_calls = 0;
			int quad_count = 0;
		};

		static const Stats& GetStats();
		static void ResetStats();
	};
}	// namespace smokey_bedrock_parser