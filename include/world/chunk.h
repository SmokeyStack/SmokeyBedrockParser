#pragma once

#include <cstdint>
#include <cstring>
#include <string>

namespace smokey_bedrock_parser {
	class Chunk {
	public:
		int32_t chunk_x, chunk_z;
		uint16_t blocks[16][16];
		int32_t chunk_format_version;

		Chunk() {
			memset(blocks, 0, sizeof(blocks));

			chunk_format_version = -1;
		}

		int32_t ParseChunk(int32_t chunk_x, int32_t chunk_y, int32_t chunk_z, const char* buffer, size_t buffer_length,
			int32_t dimension_id, const std::string& dimension_name);
	};
}