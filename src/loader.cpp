#include "loader.h"

#include <filesystem>
#include <fstream>

#include "logger.h"
#include "minecraft/block.h"

std::map<std::string, std::tuple<int, int, int>> color_map = {
	{"CLAY",std::make_tuple(164, 168, 184)},
	{"COLOR_BLACK",std::make_tuple(25, 25, 25)},
	{"COLOR_BLUE",std::make_tuple(51, 76, 178)},
	{"COLOR_BROWN",std::make_tuple(102, 76, 51)},
	{"COLOR_CYAN",std::make_tuple(76, 127, 153)},
	{"COLOR_GRAY",std::make_tuple(76, 76, 76)},
	{"COLOR_GREEN",std::make_tuple(102, 127, 51)},
	{"COLOR_LIGHT_BLUE",std::make_tuple(102, 153, 216)},
	{"COLOR_LIGHT_GRAY",std::make_tuple(153, 153, 153)},
	{"COLOR_LIGHT_GREEN",std::make_tuple(127, 204, 25)},
	{"COLOR_MAGENTA",std::make_tuple(178, 76, 216)},
	{"COLOR_ORANGE",std::make_tuple(216, 127, 51)},
	{"COLOR_PINK",std::make_tuple(242, 127, 165)},
	{"COLOR_PURPLE",std::make_tuple(127, 63, 178)},
	{"COLOR_RED",std::make_tuple(153, 51, 51)},
	{"COLOR_YELLOW",std::make_tuple(229, 229, 51)},
	{"DEEPSLATE", std::make_tuple(100, 100, 100)},
	{"DIRT",std::make_tuple(151, 109, 77)},
	{"FIRE",std::make_tuple(255, 0, 0)},
	{"GRASS", std::make_tuple(127, 178, 56)},
	{"ICE",std::make_tuple(160, 160, 255)},
	{"METAL",std::make_tuple(167, 167, 167)},
	{"NONE",std::make_tuple(50, 50, 50)},
	{"PLANT", std::make_tuple(0, 124, 0)},
	{"QUARTZ",std::make_tuple(255, 252, 245)},
	{"SAND",std::make_tuple(247, 233, 163)},
	{"SNOW",std::make_tuple(255, 255, 255)},
	{"STONE",std::make_tuple(112, 112, 112)},
	{"WATER",std::make_tuple(64, 64, 255)},
	{"WOOD",std::make_tuple(143, 119, 72)},
	{"WOOL",std::make_tuple(199, 199, 199)}
};

namespace smokey_bedrock_parser {
	int32_t LoadBlocks(nlohmann::json json) {
		log::info("Loading blocks");

		for (auto& [key, val] : json.items()) {
			for (auto& [inner_key, inner_val] : val.items()) {
				std::string name = inner_val.dump().c_str();
				std::tuple<int, int, int> color;;
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