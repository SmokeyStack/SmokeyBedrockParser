#include "world/chunk.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include "nbt.h"
#include "logger.h"
#include "json.hpp"

// Thanks to the project bedrock-viz for this math logic
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

int32_t GetBitFromByte(const char* buffer, int32_t bit_number) {
	int byte_start = bit_number / 8;
	int byte_offset = bit_number % 8;

	return buffer[byte_start] & (1 << byte_offset);
}

int32_t GetBitsFromBytes8(const char* buffer, int32_t bit_start, int32_t bit_length)
{
	unsigned byte_start = bit_start / 8;
	unsigned byte_offset = bit_start % 8;
	uint8_t byte_low = buffer[byte_start];
	uint8_t byte_high = buffer[byte_start + 1];
	uint16_t value = byte_low + (byte_high << 8u);
	value = value >> byte_offset;
	uint16_t mask = (1u << unsigned(bit_length)) - 1;

	return value & mask;
}

int32_t GetBitsFromBytesLarge(const char* buffer, int32_t bit_start, int32_t bit_length)
{
	int32_t result = 0;

	for (int b = 0; b < bit_length; b++) {
		uint8_t bit = GetBitFromByte(buffer, bit_start + b);

		if (bit) result |= 1 << b;
	}

	return result;
}

int32_t GetBitsFromBytes(const char* buffer, int32_t bit_start, int32_t bit_length)
{
	if (bit_length <= 8) return GetBitsFromBytes8(buffer, bit_start, bit_length);

	return GetBitsFromBytesLarge(buffer, bit_start, bit_length);
}

int32_t GetBlockId(const char* palette, int blocks_per_word, int bits_per_block,
	int32_t x, int32_t z, int32_t y) {
	int block_position = (((x * 16) + z) * 16) + y;
	int word_start = block_position / blocks_per_word;
	int bit_offset = (block_position % blocks_per_word) * bits_per_block;
	int bit_start = word_start * 4 * 8 + bit_offset;

	return GetBitsFromBytes(palette, bit_start, bits_per_block);
}

namespace smokey_bedrock_parser {
	int32_t Chunk::ParseChunk(int32_t chunk_x, int32_t chunk_y, int32_t chunk_z, const char* buffer, size_t buffer_length,
		int32_t dimension_id, const std::string& dimension_name) {
		// https://gist.github.com/Tomcc/a96af509e275b1af483b25c543cfbf37
		int32_t blocks_per_word = -1;
		int32_t bits_per_block = -1;
		int32_t block_offset = -1;
		int32_t palette_offset = -1;

		if (SetupBlockStorage(buffer, blocks_per_word, bits_per_block, block_offset, palette_offset) != 0)
			return -1;

		NbtTagList tag_list;
		int offset = palette_offset + 4;
		NbtJson test;
		test.name = "nbt";

		test.nbt = ParseNbt(" ", &buffer[offset], buffer_length - offset, tag_list).second;
		//log::info("{}", test.nbt.dump(4, ' ', false, nlohmann::detail::error_handler_t::ignore));

		std::vector<std::string> chunk_palette_id(tag_list.size());
		std::vector<int32_t> chunk_palette_data(tag_list.size());

		for (size_t i = 0; i < tag_list.size(); i++) {
			if (tag_list[i].second->get_type() == nbt::tag_type::Compound) {
				nbt::tag_compound tag_compound = tag_list[i].second->as<nbt::tag_compound>();

				if (tag_compound.has_key("name", nbt::tag_type::String)) {
					chunk_palette_id[i] = tag_compound["name"].as<nbt::tag_string>().get();

					// TODO: Deal with block states later
					/*if (tag_compound.has_key("states", nbt::tag_type::Compound)) {
						nbt::tag_compound states = tag_compound["states"].as<nbt::tag_compound>();
					}*/
				}
			}
		}

		int32_t palette_id, palette_data;
		std::string block_id;
		for (int32_t y = 0; y < 16; y++) {
			for (int32_t z = 0; z < 16; z++) {
				for (int32_t x = 0; x < 16; x++) {
					if (blocks_per_word != 0 && bits_per_block != 0)
						palette_id = GetBlockId(&buffer[block_offset], blocks_per_word, bits_per_block, x, z, y);
					else
						palette_id = 0;

					block_id = chunk_palette_id[palette_id];
					log::trace("Block ID: {}, (x: {}, y: {}, z: {})", block_id, x + chunk_x * 16, y + chunk_y * 16, z + chunk_z * 16);
				}
			}
		}
	}
}