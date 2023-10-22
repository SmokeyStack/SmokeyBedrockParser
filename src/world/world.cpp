#include "world/world.h"

#include <leveldb/filter_policy.h>
#include <leveldb/cache.h>
#include <leveldb/env.h>
#include <leveldb/options.h>
#include <leveldb/decompress_allocator.h>
#include "leveldb/zlib_compressor.h"

#include "logger.h"
#include "nbt.h"
#include "json.hpp"

int8_t ParseInt8(const char* p, int32_t startByte) {
	return (p[startByte] & 0xff);
}

int32_t ParseInt32(const char* p, int32_t startByte) {
	int32_t ret;
	memcpy(&ret, &p[startByte], 4);
	return ret;
}

std::pair<bool, int32_t> IsChunkKey(std::string_view key) {
	auto tag_test = [](char tag) {
		return ((33 <= tag && tag <= 64) || tag == 118);
		};

	if (key.size() == 9 || key.size() == 10)
		return std::make_pair(tag_test(key[8]), ParseInt8(key.data(), 8));
	else if (key.size() == 13 || key.size() == 14)
		return std::make_pair(tag_test(key[12]), ParseInt8(key.data(), 12));

	return std::make_pair(false, 0);
}

// https://learn.microsoft.com/en-us/minecraft/creator/documents/actorstorage
enum class ChunkTag : char {
	Data3D = 43,
	Version, // This was moved to the front as needed for the extended heights feature. Old chunks will not have this data.
	Data2D,
	Data2DLegacy,
	SubChunkPrefix,
	LegacyTerrain,
	BlockEntity,
	Entity,
	PendingTicks,
	LegacyBlockExtraData,
	BiomeState,
	FinalizedState,
	ConversionData, // data that the converter provides, that are used at runtime for things like blending
	BorderBlocks,
	HardcodedSpawners,
	RandomTicks,
	CheckSums,
	GenerationSeed,
	GeneratedPreCavesAndCliffsBlending = 61, // not used, DON'T REMOVE
	BlendingBiomeHeight = 62, // not used, DON'T REMOVE
	MetaDataHash,
	BlendingData,
	ActorDigestVersion,
	LegacyVersion = 118,
};

struct ChunkData {
	int32_t chunk_x;
	int32_t chunk_z;
	int32_t chunk_dimension_id;
	ChunkTag chunk_tag;
	int32_t chunk_type_sub;
	std::string dimension_name;
};

ChunkData ParseChunkKey(std::string_view key) {
	ChunkData chunk_data;

	chunk_data.chunk_x = key[0];
	chunk_data.chunk_z = key[4];
	chunk_data.chunk_type_sub = 0;

	switch (key.size()) {
	case 9: {
		chunk_data.chunk_dimension_id = 0;
		chunk_data.dimension_name = "overworld";
		chunk_data.chunk_tag = (ChunkTag)key[8];
	}
		  break;
	case 10: {
		chunk_data.chunk_dimension_id = 0;
		chunk_data.dimension_name = "overworld";
		chunk_data.chunk_tag = (ChunkTag)key[8];
		chunk_data.chunk_type_sub = key[9];
	}
		   break;
	case 13: {
		chunk_data.chunk_dimension_id = key[8];
		chunk_data.dimension_name = "nether";
		chunk_data.chunk_tag = (ChunkTag)key[12];

		if (chunk_data.chunk_dimension_id == 0x32373639) {
			chunk_data.chunk_dimension_id = 2;
		}
		if (chunk_data.chunk_dimension_id == 0x33373639) {
			chunk_data.chunk_dimension_id = 1;
		}

		// check for new dim id's
		if (chunk_data.chunk_dimension_id != 1 && chunk_data.chunk_dimension_id != 2) {
			smokey_bedrock_parser::log::warn("UNKNOWN -- Found new chunkDimId=0x{:x} -- Did Bedrock finally get custom dimensions? Or did Mojang add a new dimension?", chunk_data.chunk_dimension_id);
		}
	}
		   break;
	case 14: {
		chunk_data.chunk_dimension_id = key[8];
		chunk_data.dimension_name = "nether";
		chunk_data.chunk_tag = (ChunkTag)key[12];
		chunk_data.chunk_type_sub = key[13];

		if (chunk_data.chunk_dimension_id == 0x32373639) {
			chunk_data.chunk_dimension_id = 2;
		}
		if (chunk_data.chunk_dimension_id == 0x33373639) {
			chunk_data.chunk_dimension_id = 1;
		}

		// check for new dim id's
		if (chunk_data.chunk_dimension_id != 1 && chunk_data.chunk_dimension_id != 2) {
			smokey_bedrock_parser::log::warn("UNKNOWN -- Found new chunk_dimension_id=0x{:x} -- Did Bedrock finally get custom dimensions? Or did Mojang add a new dimension?", chunk_data.chunk_dimension_id);
		}
	}
		   break;
	default:
		break;
	}

	return chunk_data;
}

namespace {
	class NullLogger : public leveldb::Logger {
	public:
		void Logv(const char*, va_list) override {}
	};
}

namespace smokey_bedrock_parser {
	MinecraftWorldLevelDB::MinecraftWorldLevelDB() {
		db = nullptr;
		db_options = std::make_unique<leveldb::Options>();

		// create a bloom filter to quickly tell if a key is in the database or
		// not
		db_options->filter_policy = leveldb::NewBloomFilterPolicy(10);

		// create a 40 mb cache (we use this on ~1gb devices)
		db_options->block_cache = leveldb::NewLRUCache(40 * 1024 * 1024);

		// create a 4mb write buffer, to improve compression and touch the disk
		// less
		db_options->write_buffer_size = 4 * 1024 * 1024;

		// disable internal logging. The default logger will still print out
		// things to a file
		db_options->info_log = new NullLogger();

		// use the new raw-zip compressor to write (and read)
		auto zlib_raw_compressor = std::make_unique<leveldb::ZlibCompressorRaw>(-1);
		db_options->compressors[0] = zlib_raw_compressor.get();

		// also setup the old, slower compressor for backwards compatibility.
		// This will only be used to read old compressed blocks.
		auto zlib_compressor = std::make_unique<leveldb::ZlibCompressor>();
		db_options->compressors[1] = zlib_compressor.get();

		// create a reusable memory space for decompression so it allocates less
		leveldb::ReadOptions readOptions;
		readOptions.decompress_allocator = new leveldb::DecompressAllocator();

		for (int32_t i = 0; i < 3; i++) {
			dimensions.push_back(std::make_unique<Dimension>());
			dimensions[i]->set_dimension_id(i);
			dimensions[i]->set_dimension_name(dimension_id_names[i]);
		}
	}

	int32_t MinecraftWorldLevelDB::ParseLevelFile(std::string file_name) {
		FILE* file = fopen(file_name.c_str(), "rb");
		if (!file) {
			log::error("Failed to open input file (file name={} error={} ({}))", file_name, strerror(errno), errno);

			return -1;
		}

		int32_t format_version;
		int32_t buffer_length;
		fread(&format_version, sizeof(int32_t), 1, file);
		fread(&buffer_length, sizeof(int32_t), 1, file);

		log::info("ParseLevelFile: name={} version={} length={}", file_name, format_version, buffer_length);

		int32_t result = -2;

		if (buffer_length > 0) {
			char* buffer = new char[buffer_length];

			fread(buffer, 1, buffer_length, file);
			fclose(file);

			NbtTagList tag_list;

			result = ParseNbt("level.dat: ", buffer, buffer_length, tag_list).first;

			if (result == 0) {
				nbt::tag_compound tag_compound = tag_list[0].second->as<nbt::tag_compound>();

				set_world_spawn_x(tag_compound["SpawnX"].as<nbt::tag_int>().get());

				set_world_spawn_y(tag_compound["SpawnY"].as<nbt::tag_int>().get());

				set_world_spawn_z(tag_compound["SpawnZ"].as<nbt::tag_int>().get());

				log::info("Found World Spawn: x={} y={} z={}",
					get_world_spawn_x(),
					get_world_spawn_y(),
					get_world_spawn_z());

				set_world_seed(tag_compound["RandomSeed"].as<nbt::tag_long>().get());
			}

			delete[] buffer;
		}
		else {
			fclose(file);
		}

		return result;
	}

	int32_t MinecraftWorldLevelDB::init(std::string db_directory) {
		int32_t result = ParseLevelFile(std::string(db_directory + "/level.dat"));

		if (result != 0) {
			log::error("Failed to parse level.dat file.  Exiting...");

			return -1;
		}

		result = ParseLevelName(std::string(db_directory + "/levelname.txt"));

		if (result != 0)
			log::warn("WARNING: Failed to parse levelname.txt file.");

		return 0;
	}

	int32_t MinecraftWorldLevelDB::dbOpen(std::string db_directory) {
		log::info("DB Open: dir={}", db_directory);
		db_options = std::make_unique<leveldb::Options>();
		leveldb::Status status = leveldb::DB::Open(*db_options, std::string(db_directory + "/db").c_str(), &db);
		log::info("DB Open Status: {}", status.ToString());

		if (!status.ok())
			log::error("LevelDB operation returned status={}", status.ToString());

		return 0;
	}

	int32_t MinecraftWorldLevelDB::CalculateTotalRecords() {
		int32_t record_count = 0;
		leveldb::Slice key, value;

		leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());

		for (it->SeekToFirst(); it->Valid(); it->Next())
			record_count++;

		total_record_count = record_count;

		return 0;
	}

	int32_t MinecraftWorldLevelDB::dbParse() {
		char temp_string[256];
		log::info("Parsing all leveldb records");

		CalculateTotalRecords();

		NbtTagList tag_list;
		int32_t record_count = 0, result;
		leveldb::Slice key, value;
		size_t key_size;
		size_t value_size;
		const char* key_name;
		const char* key_data;
		std::string chunk_string;
		std::vector<std::string> villages;
		std::vector<uint64_t> actor_ids;
		leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());

		for (it->SeekToFirst(); it->Valid(); it->Next()) {
			key = it->key();
			key_size = (int)key.size();
			key_name = key.data();
			value = it->value();
			value_size = value.size();
			key_data = value.data();
			record_count++;

			if ((record_count % 100) == 0) {
				double percentage = (double)record_count / (double)total_record_count;
				log::info("Processing records: {} / {} ({:.1f}%)", record_count, total_record_count, percentage * 100.0);
			}

			/**
				Sources for keys: https://minecraft.wiki/w/Bedrock_Edition_level_format
				Sources for more NBT data: https://minecraft.wiki/w/Bedrock_Edition_level_format/Other_data_format
			*/;
			if (strncmp(key_name, "BiomeData", key_size) == 0) {
				log::info("Found key - BiomeData");
				ParseNbt("BiomeData: ", key_data, int32_t(value_size), tag_list);
			}
			else if (strncmp(key_name, "Overworld", key_size) == 0) {
				log::info("Found key - Overworld");
				ParseNbt("Overworld: ", key_data, int32_t(value_size), tag_list);
			}
			else if (strncmp(key_name, "~local_player", key_size) == 0) {
				log::info("Found key - ~local_player");
				ParseNbt("Local Player: ", key_data, int32_t(value_size), tag_list);
			}
			else if ((key_size >= 7) && (strncmp(key_name, "player_", 7) == 0)) {
				std::string player_remote_Id = std::string(&key_name[strlen("player_")], key_size - strlen("player_"));
				log::info("Found key - player_{}", player_remote_Id);
				ParseNbt("Remote Player: ", key_data, int32_t(key_data), tag_list);
			}
			else if (strncmp(key_name, "game_flatworldlayers", key_size) == 0) {
				log::info("Found key - game_flatworldlayers");
				ParseNbt("game_flatworldlayers: ", key_data, int32_t(value_size), tag_list);
			}
			/*else if (strncmp(key_name, "VILLAGE_", 8) == 0) {
				log::info("Village-{}",key_name);
				char village_id[37];
				char rectype[9];
				memcpy(village_id, key_name + 8, 36);
				log::info("Village ID-{}", village_id);

				village_id[36] = '\0';
				memcpy(rectype, key_name + 45, key_size - 45);
				log::info("Village RecType-{}, {}", rectype, rectype[key_size - 45]);
				rectype[key_size - 45] = 'h';
				if (strncmp(rectype, "INFO", 5) == 0) {
					villages.push_back(village_id);
				}
			}*/
			else if (strncmp(key_name, "AutonomousEntities", key_size) == 0) {
				log::info("Found key - AutonomousEntities");
				ParseNbt("AutonomousEntities: ", key_data, int32_t(value_size), tag_list);
			}
			else if (strncmp(key_name, "digp", 4) == 0) {
				for (uint32_t i = 0; i < value_size; i += 8) {
					actor_ids.push_back(*(uint64_t*)(key_data + i));
				}
			}
			else if (strncmp(key_name, "actorprefix", 11) == 0) {
				log::info("Found key - actorprefix");
				ParseNbt("actorprefix: ", key_data, int32_t(value_size), tag_list);
			}
			else if (IsChunkKey({ key_name,key_size }).first) {
				ChunkData chunk_data = ParseChunkKey({ key_name, key_size });
				chunk_string = chunk_data.dimension_name + "-chunk: ";
				sprintf(temp_string, "%d %d (type=0x%02x) (subtype=0x%02x) (size=%d)", chunk_data.chunk_x, chunk_data.chunk_z, chunk_data.chunk_tag,
					chunk_data.chunk_type_sub, (int32_t)value_size);
				chunk_string += temp_string;
				log::info("{}", chunk_string);

				switch (chunk_data.chunk_tag) {
				case ChunkTag::SubChunkPrefix: {
					if (key_data[0] != 0)
						dimensions[chunk_data.chunk_dimension_id]->AddChunk(7, chunk_data.chunk_x, chunk_data.chunk_type_sub, chunk_data.chunk_z, key_data, value_size);
				}
											 break;
				default:
					break;
				}
			}
			else {
				log::info("Unknown record - key_size={} value_size={}", key_size, value_size);

			}
		}

		NbtJson test;
		test.name = "nbt";

		for (auto actor_id : actor_ids) {
			std::string data;
			char key[19] = "actorprefix";

			memcpy(key + 11, &actor_id, 8);

			NbtTagList actor_list;
			leveldb::ReadOptions read_options;

			db->Get(read_options, leveldb::Slice(key, 19), &data);

			log::info("{}", ParseNbt("actorprefix: ", data.data(), data.size(), actor_list).second[0].dump(4, ' ', false, nlohmann::detail::error_handler_t::ignore));
		}

		/*
		for (auto village_id : villages) {
			std::string data;
			NbtTagList village_list;
			leveldb::ReadOptions read_options;
			db->Get(read_options, ("VILLAGE_" + village_id + "_INFO"), &data);
			ParseNbt("village_info: ", data.data(), data.size(), village_list);
			db->Get(read_options, ("VILLAGE_" + village_id + "_PLAYERS"), &data);
			ParseNbt("village_info: ", data.data(), data.size(), village_list);
			db->Get(read_options, ("VILLAGE_" + village_id + "_DWELLERS"), &data);
			ParseNbt("village_info: ", data.data(), data.size(), village_list);
			db->Get(read_options, ("VILLAGE_" + village_id + "_POI"), &data);
			ParseNbt("village_info: ", data.data(), data.size(), village_list);
		}
		*/

		log::info("Read {} records", record_count);
		log::info("Status: {}", it->status().ToString());

		if (!it->status().ok())
			log::warn("LevelDB operation returned status={}", it->status().ToString());

		delete it;

		return 0;
	}

	std::unique_ptr<MinecraftWorldLevelDB> world;
}