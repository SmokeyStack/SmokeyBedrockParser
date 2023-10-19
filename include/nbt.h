#pragma once

#include "nbt_tags.h"

namespace smokey_bedrock_parser {
	class PlayerInfo {
	public:
		int64_t unique_id;
		std::string player_id;
	};

	typedef std::pair<std::string, std::unique_ptr<nbt::tag>> NbtTag;
	typedef std::vector<NbtTag> NbtTagList;

	int32_t ParseNbt(char header, char buffer, int32_t buffer_length, NbtTagList tag_list);
}