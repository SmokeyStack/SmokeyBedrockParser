#include <iostream>
#include <tuple>

#include "nbt.h"
#include "logger.h"
#include "json.hpp"

std::string makeIndent(int32_t indent, const char* hdr) {
	std::string s;
	s.append(hdr);
	for (int32_t i = 0; i < indent; i++) {
		s.append("  ");
	}
	return s;
}

namespace smokey_bedrock_parser {
	int32_t global_nbt_list_number = 0;
	int32_t global_nbt_compound_number = 0;

	nlohmann::json::object_t ParseNbtTag(const char* header, int& indent, const NbtTag& tag) {
		log::trace("{}NBT Tag: {}", makeIndent(indent, header), tag.first);

		nlohmann::json::object_t json;

		nbt::tag_type nbt_type = tag.second->get_type();

		switch (nbt_type) {
		case nbt::tag_type::End:
			log::trace("TAG_END");
			break;
		case nbt::tag_type::Byte: {
			nbt::tag_byte value = tag.second->as<nbt::tag_byte>();
			log::trace("TAG_BYTE: {}", value.get());
			json[tag.first] = value.get();
		}
								break;
		case nbt::tag_type::Short: {
			nbt::tag_short value = tag.second->as<nbt::tag_short>();
			log::trace("TAG_SHORT: {}", value.get());
			json[tag.first] = value.get();
		}
								 break;
		case nbt::tag_type::Int: {
			nbt::tag_int value = tag.second->as<nbt::tag_int>();
			log::trace("TAG_INT: {}", value.get());
			json[tag.first] = value.get();
		}
							   break;
		case nbt::tag_type::Long: {
			nbt::tag_long value = tag.second->as<nbt::tag_long>();
			log::trace("TAG_LONG: {}", value.get());
			json[tag.first] = value.get();
		}
								break;
		case nbt::tag_type::Float: {
			nbt::tag_float value = tag.second->as<nbt::tag_float>();
			log::trace("TAG_FLOAT: {}", value.get());
			json[tag.first] = value.get();
		}
								 break;
		case nbt::tag_type::Double: {
			nbt::tag_double value = tag.second->as<nbt::tag_double>();
			log::trace("TAG_DOUBLE: {}", value.get());
			json[tag.first] = value.get();
		}
								  break;
		case nbt::tag_type::Byte_Array:
			break;
		case nbt::tag_type::String: {
			nbt::tag_string value = tag.second->as<nbt::tag_string>();
			log::trace("TAG_STRING: {}", value.get());
			json[tag.first] = value.get();
		}
								  break;
		case nbt::tag_type::List: {
			nbt::tag_list value = tag.second->as<nbt::tag_list>();
			int32_t list_number = global_nbt_compound_number++;
			log::trace("LIST-{} {{", list_number);
			indent++;

			for (const auto& nbt_tag : value) {
				json[tag.first].update(ParseNbtTag(header, indent, std::make_pair(std::string(""), nbt_tag.get().clone())));
			}

			if (--indent < 0)
				indent = 0;

			log::trace("{}}} LIST-{}", makeIndent(indent, header), list_number);
		}
								break;
		case nbt::tag_type::Compound: {
			nbt::tag_compound value = tag.second->as<nbt::tag_compound>();
			int32_t compound_number = global_nbt_compound_number++;
			log::trace("TAG_COMPOUND: {} ({} tags)", compound_number, value.size());
			indent++;

			for (const auto& nbt_tag : value) {
				json[tag.first].update(ParseNbtTag(header, indent, std::make_pair(nbt_tag.first, nbt_tag.second.get().clone())));
			}

			if (indent-- < 0)
				indent = 0;

			log::trace("{}}} COMPOUND-{}", makeIndent(indent, header), compound_number);
		}
									break;
		case nbt::tag_type::Int_Array:
			break;
		case nbt::tag_type::Long_Array:
			break;
		default:
			break;
		}

		return json;
	}

	std::pair<int32_t, nlohmann::json> ParseNbt(const char* header, const char* buffer, int32_t buffer_length, NbtTagList& tag_list) {
		int32_t indent = 0;
		log::trace("{}NBT Decode Start", makeIndent(indent, header));
		global_nbt_list_number = 0;
		global_nbt_compound_number = 0;
		std::istringstream iss(std::string(buffer, buffer_length));
		nbt::io::stream_reader reader(iss, endian::little);
		tag_list.clear();
		NbtTag tag;
		bool done = false;
		std::istream& stream = reader.get_istr();

		while (!done && stream && !stream.eof()) {
			try
			{
				tag_list.push_back(reader.read_tag());
			}
			catch (const std::exception&)
			{
				if (!stream.eof())
					log::error("Failed to read tag");

				done = true;
			}
		}

		NbtJson nbt_json;
		nbt_json.name = "nbt";

		for (const auto& nbt_tag : tag_list) {
			nbt_json.nbt.push_back(ParseNbtTag(header, indent, nbt_tag));
		}

		log::trace("{}NBT Decode End ({} tags)", makeIndent(indent, header), tag_list.size());
		//log::info("{}", nbt_json.nbt.dump(4, ' ', false, nlohmann::detail::error_handler_t::ignore));

		return std::make_pair(0, nbt_json.nbt);
	}

	class VillageInfo {
	public:
		nlohmann::json village_payload;
		std::tuple<int32_t, int32_t, int32_t> position;
		int32_t x0;
		int32_t x1;
		int32_t y0;
		int32_t y1;
		int32_t z0;
		int32_t z1;
		std::map<int64_t, int32_t> players;
		std::vector<std::tuple<int32_t, int32_t, int32_t>> dwellers;
		std::vector<std::tuple<int32_t, int32_t, int32_t>> villagers;
		std::vector<std::tuple<int32_t, int32_t, int32_t>> golems;
		std::vector<std::tuple<int32_t, int32_t, int32_t>> raiders;
		std::vector<std::tuple<int32_t, int32_t, int32_t>> cats;

		VillageInfo() {
			clear();
		}

		void clear()
		{
			players.clear();
			x0 = x1 = y0 = y1 = z0 = z1 = 0;
		}

		// https://minecraft.wiki/w/Bedrock_Edition_level_format/Other_data_format#VILLAGE_[0-9a-f\\-]+_INFO
		int32_t set_info(nbt::tag_compound& tag) {
			x0 = tag["X0"].as<nbt::tag_int>().get();
			x1 = tag["X1"].as<nbt::tag_int>().get();
			y0 = tag["Y0"].as<nbt::tag_int>().get();
			y1 = tag["Y1"].as<nbt::tag_int>().get();
			z0 = tag["Z0"].as<nbt::tag_int>().get();
			z1 = tag["Z1"].as<nbt::tag_int>().get();
			position = std::make_tuple((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2);
			village_payload["Village Info"] = { {"Village Center", {{"x", std::get<0>(position)},{"y", std::get<1>(position)},{"z", std::get<2>(position)}}} };

			return 0;
		}

		// https://minecraft.wiki/w/Bedrock_Edition_level_format/Other_data_format#VILLAGE_[0-9a-f\\-]+_PLAYERS
		int32_t set_players(nbt::tag_compound& tag) {
			if (tag.has_key("Player", nbt::tag_type::List)) {
				nbt::tag_list player_list = tag["Player"].as<nbt::tag_list>();

				for (const auto& player : player_list) {
					nbt::tag_compound player_compound = player.as<nbt::tag_compound>();
					players[player_compound["ID"].as<nbt::tag_long>().get()] = player_compound["S"].as<nbt::tag_int>().get();
				}
			}

			log::info("{}", players.size());
			for (const auto& player : players)
				log::info("{} - {}", player.first, player.second);

			return 0;
		}

		/*
		* https://minecraft.wiki/w/Bedrock_Edition_level_format/Other_data_format#VILLAGE_[0-9a-f\\-]+_DWELLERS
		* What is TS??? Edit: Timestamp according to Github Copilot.
		* There are four types of dwellers: villagers, iron golems, raiders, and cats. Each dweller has a unique ID and a timestamp (TS).
		* Issue is that Dwellers is a list so I do not know how to get the type of dweller.
		* TODO: Figure out how to get the type of dweller.
		*/
		int32_t set_dwellers(nbt::tag_compound& tag) {
			if (tag.has_key("Dwellers", nbt::tag_type::List)) {
				nbt::tag_list dweller_list = tag["Dwellers"].as<nbt::tag_list>();
				log::info("{}", dweller_list.size());

				for (const auto& dweller : dweller_list) {
					for (const auto& dweller_compound : dweller.as<nbt::tag_compound>()) {
						nbt::tag_list dweller_tag = dweller_compound.second.as<nbt::tag_list>();
						for (const auto& value : dweller_tag) {
							nbt::tag_compound value_compound = value.as<nbt::tag_compound>();
							log::info("ID - {}", value_compound["ID"].as<nbt::tag_long>().get());
							log::info("Timestamp - {}", value_compound["TS"].as<nbt::tag_long>().get());
							nbt::tag_list last_saved_pos = value_compound["last_saved_pos"].as<nbt::tag_list>();
							log::info("Last Saved Position - {} - {} - {}", last_saved_pos[0].as<nbt::tag_int>().get(), last_saved_pos[1].as<nbt::tag_int>().get(), last_saved_pos[2].as<nbt::tag_int>().get());
							dwellers.push_back(std::make_tuple(last_saved_pos[0].as<nbt::tag_int>().get(), last_saved_pos[1].as<nbt::tag_int>().get(), last_saved_pos[2].as<nbt::tag_int>().get()));
							village_payload["Village Dwellers"].push_back({ { "ID", value_compound["ID"].as<nbt::tag_long>().get() }, {"Timestamp", value_compound["TS"].as<nbt::tag_long>().get()}, {"Last Saved Position", {last_saved_pos[0].as<nbt::tag_int>().get(), last_saved_pos[1].as<nbt::tag_int>().get(), last_saved_pos[2].as<nbt::tag_int>().get()}} });
						}
					}
				}
			}

			return 0;
		}

		// https://minecraft.wiki/w/Bedrock_Edition_level_format/Other_data_format#VILLAGE_[0-9a-f\\-]+_POI
		// TODO
		int32_t set_poi(nbt::tag_compound& tag) {
			return 0;
		}

		int32_t print_info() {
			log::info("{}", village_payload.dump(4, ' ', false, nlohmann::detail::error_handler_t::ignore));

			return 0;
		}
	};

	int32_t ParseNbtVillage(NbtTagList& tags_info, NbtTagList& tags_player, NbtTagList& tags_dweller, NbtTagList& tags_poi) {
		std::unique_ptr<VillageInfo> village_info = std::make_unique<VillageInfo>();

		village_info->clear();
		village_info->set_info(tags_info[0].second->as<nbt::tag_compound>());
		village_info->set_players(tags_player[0].second->as<nbt::tag_compound>());
		village_info->set_dwellers(tags_dweller[0].second->as<nbt::tag_compound>());
		village_info->print_info();

		return 0;
	}
}