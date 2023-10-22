#pragma once

#include <cstdio>
#include <leveldb/db.h>

#include "logger.h"
#include "world/dimension.h"


namespace smokey_bedrock_parser {
	class MinecraftWorld {
	private:
		std::string world_name;
		int64_t world_seed;
		int32_t world_spawn_x;
		int32_t world_spawn_y;
		int32_t world_spawn_z;
	public:
		MinecraftWorld() {
			world_name = "";
			world_seed = 0;
			world_spawn_x = 0;
			world_spawn_y = 0;
			world_spawn_z = 0;
		}

		void set_world_name(std::string name) {
			world_name = name;
		}

		std::string get_world_name() {
			return world_name;
		}

		void set_world_seed(int64_t seed) {
			world_seed = seed;
		}

		int64_t get_world_seed() {
			return world_seed;
		}

		void set_world_spawn_x(int32_t x) {
			world_spawn_x = x;
		}

		int32_t get_world_spawn_x() {
			return world_spawn_x;
		}

		void set_world_spawn_y(int32_t y) {
			world_spawn_y = y;
		}

		int32_t get_world_spawn_y() {
			return world_spawn_y;
		}

		void set_world_spawn_z(int32_t z) {
			world_spawn_z = z;
		}

		int32_t get_world_spawn_z() {
			return world_spawn_z;
		}
	};

	class MinecraftWorldLevelDB : public MinecraftWorld {
	private:
		leveldb::DB* db;
		std::unique_ptr<leveldb::Options> db_options;
		int32_t total_record_count;
	public:
		std::vector<std::unique_ptr<Dimension>> dimensions;

		MinecraftWorldLevelDB();

		~MinecraftWorldLevelDB() {
			dbClose();
		}

		int32_t ParseLevelFile(std::string file_name);

		int32_t ParseLevelName(std::string file_name) {
			FILE* file = fopen(file_name.c_str(), "r");

			if (!file) {
				log::error("Failed to open input file (fn={} error={} ({}))", file_name, strerror(errno), errno);

				return -1;
			}

			char buffer[1025];

			memset(buffer, 0, 1025);

			fgets(buffer, 1024, file);

			set_world_name(buffer);

			log::info("levelname.txt: Level name is '{}'", strlen(buffer) > 0 ? buffer : "(UNKNOWN)");

			fclose(file);

			return 0;
		}

		int32_t init(std::string db_directory);

		int32_t dbOpen(std::string db_directory);

		int32_t dbClose() {
			if (db != nullptr) {
				delete db;
				db = nullptr;
			}
			return 0;
		}

		int32_t CalculateTotalRecords();

		int32_t dbParse();
	};

	extern std::unique_ptr<MinecraftWorldLevelDB> world;

} // namespace smokey_bedrock_parser