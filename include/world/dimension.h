#pragma once

#include <climits>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "logger.h"
#include "world/chunk.h"

namespace smokey_bedrock_parser {
	const std::vector<std::string> dimension_id_names{ "overworld","nether","the-end" };

	class Dimension {
	public:
		Dimension() {
			dimension_name = "(UNKNOWN)";
			dimension_id = -1;
			min_chunk_x = INT32_MAX;
			max_chunk_x = INT32_MIN;
			min_chunk_z = INT32_MAX;
			max_chunk_z = INT32_MIN;
		}

		void set_dimension_name(std::string name) {
			dimension_name = name;
		}

		std::string get_dimension_name() {
			return dimension_name;
		}

		void set_dimension_id(int32_t id) {
			dimension_id = id;
		}

		int32_t get_dimension_id() {
			return dimension_id;
		}

		int32_t get_min_chunk_x() {
			return min_chunk_x;
		}

		int32_t get_max_chunk_x() {
			return max_chunk_x;
		}

		int32_t get_min_chunk_z() {
			return min_chunk_z;
		}

		int32_t get_max_chunk_z() {
			return max_chunk_z;
		}

		int32_t AddChunk(int32_t chunk_format_version, int32_t chunk_x, int32_t chunk_y, int32_t chunk_z, const char* buffer,
			size_t buffer_length) {
			ChunkKey key(chunk_x, chunk_z);

			if (chunk_format_version == 7)
			{
				if (!chunks_has_key(chunks, key))
					chunks[key] = std::unique_ptr<Chunk>(new Chunk());


				chunks[key]->ParseChunk(chunk_x, chunk_y, chunk_z, buffer, buffer_length, dimension_id, dimension_name);
				for (int32_t i = 0; i < 16; i++)
					for (int32_t a = 0; a < 16; a++)
						log::trace("Block: {} at Y={}", chunks[key]->blocks[i][a], chunks[key]->top_blocks[i][a]);



				return 0;
			}
			else {
				log::error("Unknown chunk format version (version = {})", chunk_format_version);
				return -1;
			}
		};

		int32_t GetChunk(int32_t chunk_x, int32_t chunk_y, int32_t chunk_z) {
			ChunkKey key(chunk_x, chunk_z);

			log::info("Block: {} at Y={}", chunks[key]->blocks[chunk_x][chunk_z], chunks[key]->top_blocks[chunk_x][chunk_z]);

			return 0;
		};

	private:
		std::string dimension_name;
		int32_t dimension_id;
		typedef std::pair<uint32_t, uint32_t> ChunkKey;
		typedef std::map<ChunkKey, std::unique_ptr<Chunk>> ChunkMap;
		ChunkMap chunks;
		int32_t min_chunk_x, max_chunk_x;
		int32_t min_chunk_z, max_chunk_z;

		bool chunks_has_key(const ChunkMap& m, const ChunkKey& k) {
			return m.find(k) != m.end();
		}
	};
} // namespace smokey_bedrock_parser