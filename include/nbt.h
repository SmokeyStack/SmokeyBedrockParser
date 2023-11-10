#pragma once

#include "nbt_tags.h"
#include "json.hpp"

namespace smokey_bedrock_parser {
	typedef struct NbtJson {
		std::string name;
		nlohmann::json nbt;
	};

	class PlayerInfo {
	public:
		int64_t unique_id;
		std::string player_id;
	};

	typedef std::pair<std::string, std::unique_ptr<nbt::tag>> NbtTag;
	typedef std::vector<NbtTag> NbtTagList;

	std::pair<int32_t, nlohmann::json> ParseNbt(const char* header, const char* buffer, int32_t buffer_length, NbtTagList& tag_list);
	int32_t ParseNbtVillage(NbtTagList& tags_info, NbtTagList& tags_player, NbtTagList& tags_dweller, NbtTagList& tags_poi);
}