#include "world/world.h"

#include <leveldb/filter_policy.h>
#include <leveldb/cache.h>
#include <leveldb/env.h>
#include <leveldb/options.h>
#include <leveldb/decompress_allocator.h>
#include "leveldb/zlib_compressor.h"

#include "logger.h"

namespace {
	class NullLogger : public leveldb::Logger {
	public:
		void Logv(const char*, va_list) override {}
	};
}

namespace smokey_bedrock_parser {
	MinecraftWorldLevelDB::MinecraftWorldLevelDB() {
		db = nullptr;
		leveldb::Options options;

		// create a bloom filter to quickly tell if a key is in the database or
		// not
		options.filter_policy = leveldb::NewBloomFilterPolicy(10);

		// create a 40 mb cache (we use this on ~1gb devices)
		options.block_cache = leveldb::NewLRUCache(40 * 1024 * 1024);

		// create a 4mb write buffer, to improve compression and touch the disk
		// less
		options.write_buffer_size = 4 * 1024 * 1024;

		// disable internal logging. The default logger will still print out
		// things to a file
		options.info_log = new NullLogger();

		// use the new raw-zip compressor to write (and read)
		auto zlib_raw_compressor = std::make_unique<leveldb::ZlibCompressorRaw>(-1);
		options.compressors[0] = zlib_raw_compressor.get();

		// also setup the old, slower compressor for backwards compatibility.
		// This will only be used to read old compressed blocks.
		auto zlib_compressor = std::make_unique<leveldb::ZlibCompressor>();
		options.compressors[1] = zlib_compressor.get();

		// create a reusable memory space for decompression so it allocates less
		leveldb::ReadOptions readOptions;
		readOptions.decompress_allocator = new leveldb::DecompressAllocator();
	}

	int32_t MinecraftWorldLevelDB::ParseLevelFile(std::string file_name) {

		FILE* fp = fopen(file_name.c_str(), "rb");
		if (!fp) {
			log::error("Failed to open input file (fn={} error={} ({}))", file_name, strerror(errno), errno);
			return -1;
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

	std::unique_ptr<MinecraftWorldLevelDB> world;
}