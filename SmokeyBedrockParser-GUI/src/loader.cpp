#include "loader.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <SmokeyBedrockParser-Core/logger.h>
#include <SmokeyBedrockParser-Core/minecraft/block.h>

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
	{"CRIMSON_HYPHAE",std::make_tuple(92, 25, 29)},
	{"CRIMSON_NYLIUM",std::make_tuple(189, 48, 49)},
	{"CRIMSON_STEM",std::make_tuple(148, 63, 97)},
	{"DEEPSLATE", std::make_tuple(100, 100, 100)},
	{"DIAMOND",std::make_tuple(92, 219, 213)},
	{"DIRT",std::make_tuple(151, 109, 77)},
	{"EMERALD",std::make_tuple(0, 217, 58)},
	{"FIRE",std::make_tuple(255, 0, 0)},
	{"GLOW_LICHEN",std::make_tuple(127, 167, 150)},
	{"GOLD",std::make_tuple(250, 238, 77)},
	{"GRASS", std::make_tuple(127, 178, 56)},
	{"ICE",std::make_tuple(160, 160, 255)},
	{"LAPIS",std::make_tuple(74, 128, 255)},
	{"METAL",std::make_tuple(167, 167, 167)},
	{"NETHER",std::make_tuple(112, 2, 0)},
	{"NONE",std::make_tuple(50, 50, 50)},
	{"PLANT", std::make_tuple(0, 124, 0)},
	{"PODZOL",std::make_tuple(129, 86, 49)},
	{"QUARTZ",std::make_tuple(255, 252, 245)},
	{"RAW_IRON",std::make_tuple(216, 175, 147)},
	{"SAND",std::make_tuple(247, 233, 163)},
	{"SNOW",std::make_tuple(255, 255, 255)},
	{"STONE",std::make_tuple(112, 112, 112)},
	{"TERRACOTTA_BLACK",std::make_tuple(37, 22, 16)},
	{"TERRACOTTA_BLUE",std::make_tuple(76, 62, 92)},
	{"TERRACOTTA_BROWN",std::make_tuple(76, 50, 35)},
	{"TERRACOTTA_CYAN",std::make_tuple(87, 92, 92)},
	{"TERRACOTTA_GRAY",std::make_tuple(57, 41, 35)},
	{"TERRACOTTA_GREEN",std::make_tuple(76, 82, 42)},
	{"TERRACOTTA_LIGHT_BLUE",std::make_tuple(112, 108, 138)},
	{"TERRACOTTA_LIGHT_GRAY",std::make_tuple(135, 107, 98)},
	{"TERRACOTTA_LIGHT_GREEN",std::make_tuple(103, 117, 53)},
	{"TERRACOTTA_MAGENTA",std::make_tuple(149, 87, 108)},
	{"TERRACOTTA_ORANGE",std::make_tuple(159, 82, 36)},
	{"TERRACOTTA_PINK",std::make_tuple(160, 77, 78)},
	{"TERRACOTTA_PURPLE",std::make_tuple(122, 73, 88)},
	{"TERRACOTTA_RED",std::make_tuple(142, 60, 46)},
	{"TERRACOTTA_WHITE",std::make_tuple(209, 177, 161)},
	{"TERRACOTTA_YELLOW",std::make_tuple(186, 133, 36)},
	{"WARPED_HYPHAE",std::make_tuple(86, 44, 62)},
	{"WARPED_NYLIUM",std::make_tuple(22, 126, 134)},
	{"WARPED_STEM",std::make_tuple(58, 142, 140)},
	{"WARPED_WART_BLOCK",std::make_tuple(20, 180, 133)},
	{"WATER",std::make_tuple(64, 64, 255)},
	{"WOOD",std::make_tuple(143, 119, 72)},
	{"WOOL",std::make_tuple(199, 199, 199)}
};


int LoadBlocks(nlohmann::json json) {
	smokey_bedrock_parser::log::info("Loading blocks");

	for (auto& [key, val] : json.items()) {
		for (auto& [inner_key, inner_val] : val.items()) {
			std::string name = inner_val.dump().c_str();
			std::tuple<int, int, int> color;;
			name = name.substr(1, name.size() - 2);

			if (color_map.find(key) != color_map.end())
				color = color_map[key];

			auto block = smokey_bedrock_parser::Block::Add(name);
		}
	}

	smokey_bedrock_parser::log::info("Loaded blocks");

	return 0;
}

int LoadJson(const std::string& path) {
	smokey_bedrock_parser::log::info("Loading JSON file: {}", path);
	std::string filepath = std::filesystem::current_path().string();
	std::ifstream file(filepath + "/" + path);

	if (!file.is_open()) {
		smokey_bedrock_parser::log::error("Failed to open file: {}", path);
		return 1;
	}

	nlohmann::json json;
	file >> json;
	smokey_bedrock_parser::log::info("Loaded JSON file: {}", path);

	LoadBlocks(json);

	return 0;
}