#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <cstdint>
#include <climits>
#include <utility>

#include "logger.h"
#include "world/chunk.h"

namespace smokey_bedrock_parser {
	const std::vector<std::string> dimension_id_names{ "overworld","nether","the-end" };

	class Dimension {
	private:
		std::string dimension_name;
		int32_t dimension_id;
		typedef std::pair<uint32_t, uint32_t> ChunkKey;
		typedef std::map<ChunkKey, std::unique_ptr<Chunk>> ChunkMap;
		ChunkMap chunks;

		int32_t min_chunk_x, max_chunk_x;
		int32_t min_chunk_z, max_chunk_z;

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
			if (chunk_format_version == 7) {
				return chunks[key]->ParseChunk(chunk_x, chunk_y, chunk_z, buffer, buffer_length, dimension_id, dimension_name);
			}
			else {
				log::error("Unknown chunk format version (version = {})", chunk_format_version);
				return -1;
			}
		};
	};
}