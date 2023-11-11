#include "world/chunk.h"

#include <string>
#include <cstring>
#include <cstdint>
#include <cmath>

#include "json.hpp"
#include "logger.h"
#include "nbt.h"

int32_t
SetupBlockStorage(const char* buffer, int32_t& blocks_per_word, int32_t& bits_per_block, int32_t& block_offset,
	int32_t& palette_offset) {
	int32_t version = -1;
	int32_t word_count = -1;

	// Check sub-chunk version
	switch (buffer[0]) {
	case 0x01:
		// v1 - [version:byte][block storage]
		version = buffer[1];
		block_offset = 1;

		break;
	case 0x08:
		// v8 - [version:byte][num_storages:byte][block storage1]...[blockStorageN]
		version = buffer[2];
		block_offset = 3;

		break;
	case 0x09:
		// https://gist.github.com/Tomcc/a96af509e275b1af483b25c543cfbf37?permalink_comment_id=3901255#gistcomment-3901255
		// v9 - [version:byte][num_storages:byte][sub_chunk_index:byte][block storage1]...[blockStorageN]
		version = buffer[3];
		block_offset = 4;

		break;
	default:
		smokey_bedrock_parser::log::error("Invalid SubChunk version found ({})",
			buffer[0]);

		return -1;
	}

	switch (version) {
	case 0x00:
		// occasional 0 size bits per block
		bits_per_block = 0;
		blocks_per_word = 0;
		palette_offset = 0;

		break;
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x08:
	case 0x0a:
	case 0x0c:
	case 0x10:
	case 0x20:
		bits_per_block = version >> 1;
		blocks_per_word = floor(32.0 / bits_per_block);
		word_count = ceil(4096.0 / blocks_per_word);
		palette_offset = word_count * 4 + block_offset;

		break;
	default:
		smokey_bedrock_parser::log::error("Unknown SubChunk palette value (value = {})", version);

		return -1;
	}

	return 0;
}

namespace smokey_bedrock_parser {
	int32_t Chunk::ParseChunk(int32_t chunk_x, int32_t chunk_y, int32_t chunk_z, const char* buffer, size_t buffer_length,
		int32_t dimension_id, const std::string& dimension_name) {
		// https://gist.github.com/Tomcc/a96af509e275b1af483b25c543cfbf37
		int32_t blocks_per_word = -1;
		int32_t bits_per_block = -1;
		int32_t block_offset = -1;
		int32_t palette_offset = -1;

		if (SetupBlockStorage(buffer, blocks_per_word, bits_per_block, block_offset, palette_offset) != 0) return -1;

		NbtTagList tag_list;
		int offset = palette_offset + 4;
		NbtJson test;
		test.name = "nbt";

		test.nbt = ParseNbt(" ", &buffer[offset], buffer_length - offset, tag_list).second;

		for (size_t i = 0; i < tag_list.size(); i++) {
			if (tag_list[i].second->get_type() == nbt::tag_type::Compound) {
				nbt::tag_compound tag_compound = tag_list[i].second->as<nbt::tag_compound>();
				if (tag_compound.has_key("name", nbt::tag_type::String)) {
					//log::info("{}", test.nbt.dump(4, ' ', false, nlohmann::detail::error_handler_t::ignore));
				}
			}
		}
	}
} // namespace smokey_bedrock_parser