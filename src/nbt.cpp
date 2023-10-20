#include <iostream>

#include "nbt.h"
#include "logger.h"

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

	int32_t ParseNbtTag(const char* header, int& indent, const NbtTag& tag) {
		log::info("{}NBT Tag: {}", makeIndent(indent, header), tag.first);

		nbt::tag_type nbt_type = tag.second->get_type();

		switch (nbt_type) {
		case nbt::tag_type::End:
			log::info("TAG_END");
			break;
		case nbt::tag_type::Byte: {
			nbt::tag_byte value = tag.second->as<nbt::tag_byte>();
			log::info("TAG_BYTE: {}", value.get());
		}
								break;
		case nbt::tag_type::Short: {
			nbt::tag_short value = tag.second->as<nbt::tag_short>();
			log::info("TAG_SHORT: {}", value.get());
		}
								 break;
		case nbt::tag_type::Int: {
			nbt::tag_int value = tag.second->as<nbt::tag_int>();
			log::info("TAG_INT: {}", value.get());
		}
							   break;
		case nbt::tag_type::Long: {
			nbt::tag_long value = tag.second->as<nbt::tag_long>();
			log::info("TAG_LONG: {}", value.get());
		}
								break;
		case nbt::tag_type::Float: {
			nbt::tag_float value = tag.second->as<nbt::tag_float>();
			log::info("TAG_FLOAT: {}", value.get());
		}
								 break;
		case nbt::tag_type::Double: {
			nbt::tag_double value = tag.second->as<nbt::tag_double>();
			log::info("TAG_DOUBLE: {}", value.get());
		}
								  break;
		case nbt::tag_type::Byte_Array:
			break;
		case nbt::tag_type::String: {
			nbt::tag_string value = tag.second->as<nbt::tag_string>();
			log::info("TAG_STRING: {}", value.get());
		}
								  break;
		case nbt::tag_type::List:
			break;
		case nbt::tag_type::Compound: {
			nbt::tag_compound value = tag.second->as<nbt::tag_compound>();
			int32_t compound_number = global_nbt_compound_number++;
			log::info("TAG_COMPOUND: {} ({} tags)", compound_number, value.size());
			indent++;
			for (const auto& nbt_tag : value) {
				ParseNbtTag(header, indent, std::make_pair(nbt_tag.first, nbt_tag.second.get().clone()));
			}
			if (indent-- < 0)
				indent = 0;
			log::info("{}}} COMPOUND-{}", makeIndent(indent, header), compound_number);
		}
									break;
		case nbt::tag_type::Int_Array:
			break;
		case nbt::tag_type::Long_Array:
			break;
		default:
			break;
		}
	}

	int32_t ParseNbt(const char* header, const char* buffer, int32_t buffer_length, NbtTagList& tag_list) {
		int32_t indent = 0;

		log::info("{}NBT Decode Start", makeIndent(indent, header));

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

		for (const auto& nbt_tag : tag_list) {
			ParseNbtTag(header, indent, nbt_tag);
		}
		log::info("{}NBT Decode End ({} tags)", makeIndent(indent, header), tag_list.size());

		return 0;
	}
}