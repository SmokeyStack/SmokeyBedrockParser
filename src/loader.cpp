#include "loader.h"

#include <filesystem>
#include <fstream>

#include "logger.h"
#include "minecraft/block.h"

std::map<std::string, std::tuple<int, int, int>> color_map = {
	{"DEEPSLATE", std::make_tuple(100, 100, 100)}
};

namespace smokey_bedrock_parser {
	int32_t LoadBlocks(nlohmann::json json) {
		log::info("Loading blocks");

		for (auto& [key, val] : json.items()) {
			for (auto& [inner_key, inner_val] : val.items()) {
				std::string name = inner_val.dump().c_str();
				std::tuple<int, int, int> color;
				name = name.substr(1, name.size() - 2);

				if (color_map.find(key) != color_map.end())
					color = color_map[key];

				auto block = Block::Add(name, color);
			}
		}

		return 0;
	}

	int32_t LoadJson(const std::string& path) {
		log::info("Loading JSON file: {}", path);
		std::string filepath = std::filesystem::current_path().string();
		std::ifstream file(filepath + "/" + path);

		if (!file.is_open()) {
			log::error("Failed to open file: {}", path);
			return 1;
		}

		nlohmann::json json;
		file >> json;
		log::info("Loaded JSON file: {}", path);

		LoadBlocks(json);

		return 0;
	}

} // namespace smokey_bedrock_parser