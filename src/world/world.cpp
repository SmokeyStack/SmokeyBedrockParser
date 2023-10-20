#include "world/world.h"

#include <leveldb/filter_policy.h>
#include <leveldb/cache.h>
#include <leveldb/env.h>
#include <leveldb/options.h>
#include <leveldb/decompress_allocator.h>
#include "leveldb/zlib_compressor.h"

#include "logger.h"
#include "nbt.h"

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
	}

	int32_t MinecraftWorldLevelDB::ParseLevelFile(std::string file_name) {

		FILE* file = fopen(file_name.c_str(), "rb");
		if (!file) {
			log::error("Failed to open input file (fn={} error={} ({}))", file_name, strerror(errno), errno);
			return -1;
		}

		int32_t format_version;
		int32_t buffer_length;
		fread(&format_version, sizeof(int32_t), 1, file);
		fread(&buffer_length, sizeof(int32_t), 1, file);

		log::info("ParseLevelFile: name={} version={} len={}", file_name, format_version, buffer_length);

		int32_t result = -2;

		if (buffer_length > 0) {
			char* buffer = new char[buffer_length];
			fread(buffer, 1, buffer_length, file);
			fclose(file);

			NbtTagList tag_list;
			result = ParseNbt("level.dat: ", buffer, buffer_length, tag_list);
		}

		return 0;

		/*

		int32_t fVersion;
		int32_t bufLen;
		fread(&fVersion, sizeof(int32_t), 1, fp);
		fread(&bufLen, sizeof(int32_t), 1, fp);

		log::info("parseLevelFile: name={} version={} len={}", fname, fVersion, bufLen);

		int32_t ret = -2;
		if (bufLen > 0) {
			// read content
			char* buf = new char[bufLen];
			fread(buf, 1, bufLen, fp);
			fclose(fp);

			MyNbtTagList tagList;
			ret = parseNbt("level.dat: ", buf, bufLen, tagList);

			if (ret == 0) {
				nbt::tag_compound tc = tagList[0].second->as<nbt::tag_compound>();

				setWorldSpawnX(tc["SpawnX"].as<nbt::tag_int>().get());
				setWorldSpawnY(tc["SpawnY"].as<nbt::tag_int>().get());
				setWorldSpawnZ(tc["SpawnZ"].as<nbt::tag_int>().get());
				log::info("  Found World Spawn: x={} y={} z={}",
					getWorldSpawnX(),
					getWorldSpawnY(),
					getWorldSpawnZ());

				setWorldSeed(tc["RandomSeed"].as<nbt::tag_long>().get());
			}

			delete[] buf;
		}
		else {
			fclose(fp);
		}

		return ret;
		*/
	}

	int32_t MinecraftWorldLevelDB::init(std::string db_directory) {
		int32_t ret = ParseLevelFile(std::string(db_directory + "/level.dat"));
		if (ret != 0) {
			log::error("Failed to parse level.dat file.  Exiting...");
			log::error("** Hint: --db must point to the dir which contains level.dat");
			return -1;
		}

		ret = ParseLevelName(std::string(db_directory + "/levelname.txt"));
		if (ret != 0) {
			log::warn("WARNING: Failed to parse levelname.txt file.");
			log::warn("** Hint: --db must point to the dir which contains levelname.txt");
		}

		// update dimension data
		/*for (int32_t i = 0; i < kDimIdCount; i++) {
			dimDataList[i]->setWorldInfo(getWorldName(), getWorldSpawnX(), getWorldSpawnZ(), getWorldSeed());
		}*/

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
		std::string dimension_name, chunk_string;
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
				log::info("  Processing records: {} / {} ({:.1f}%)", record_count, total_record_count, percentage * 100.0);
			}

			/**
				Sources for keys: https://minecraft.wiki/w/Bedrock_Edition_level_format
				Sources for more NBT data: https://minecraft.wiki/w/Bedrock_Edition_level_format/Other_data_format
			*/;
			if (strncmp(key_name, "BiomeData", key_size) == 0) {
				log::info("BiomeData:");
				ParseNbt("BiomeData: ", key_data, int32_t(value_size), tag_list);
			}
			else if (strncmp(key_name, "Overworld", key_size) == 0) {
				log::info("Overworld:");
				ParseNbt("Overworld: ", key_data, int32_t(value_size), tag_list);
			}
			else if (strncmp(key_name, "~local_player", key_size) == 0) {
				log::info("Local Player:");
				ParseNbt("Local Player: ", key_data, int32_t(value_size), tag_list);
			}
			else if ((key_size >= 7) && (strncmp(key_name, "player_", 7) == 0)) {
				std::string player_remote_Id = std::string(&key_name[strlen("player_")], key_size - strlen("player_"));
				log::trace("Remote Player (id={}) value:", player_remote_Id);
				ParseNbt("Remote Player: ", key_data, int32_t(key_data), tag_list);
			}
			else if (strncmp(key_name, "game_flatworldlayers", key_size) == 0) {
				log::trace("game_flatworldlayers value: (todo)");
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
				log::trace("AutonomousEntities:");
				ParseNbt("AutonomousEntities: ", key_data, int32_t(value_size), tag_list);
			}
			else if (strncmp(key_name, "digp", key_size) == 0) {
				for (uint32_t i = 0; i < value_size; i += 8) {
					actor_ids.push_back(*(uint64_t*)(key_data + i));
				}
			}
			else {
				log::info("Unknown chunk - key_size={} value_size={}", key_size, value_size);
			}
		}

		/*
		for (auto actor_id : actor_ids) {
			std::string data;
			char key[19] = "actorprefix";
			memcpy(key + 11, &actor_id, 8);
			NbtTagList actor_list;
			leveldb::ReadOptions read_options;
			db->Get(read_options, leveldb::Slice(key, 19), &data);
			ParseNbt("actorprefix: ", data.data(), data.size(), actor_list);
		}
		
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

		if (!it->status().ok()) {
			log::warn("LevelDB operation returned status={}", it->status().ToString());
		}
		delete it;

		return 0;
	}

	std::unique_ptr<MinecraftWorldLevelDB> world;
}