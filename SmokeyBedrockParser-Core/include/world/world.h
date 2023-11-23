#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <leveldb/db.h>
#include <sstream>

#include "logger.h"
#include "util.h"
#include "world/dimension.h"

namespace smokey_bedrock_parser {
    class MinecraftWorld {
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

        int64_t get_world_seed() const {
            return world_seed;
        }

        void set_world_spawn_x(int32_t x) {
            world_spawn_x = x;
        }

        int32_t get_world_spawn_x() const {
            return world_spawn_x;
        }

        void set_world_spawn_y(int32_t y) {
            world_spawn_y = y;
        }

        int32_t get_world_spawn_y() const {
            return world_spawn_y;
        }

        void set_world_spawn_z(int32_t z) {
            world_spawn_z = z;
        }

        int32_t get_world_spawn_z() const {
            return world_spawn_z;
        }

    private:
        std::string world_name;
        int64_t world_seed;
        int32_t world_spawn_x;
        int32_t world_spawn_y;
        int32_t world_spawn_z;
    };

    class MinecraftWorldLevelDB : public MinecraftWorld {
    public:
        std::vector<std::unique_ptr<Dimension>> dimensions;

        MinecraftWorldLevelDB();

        ~MinecraftWorldLevelDB() {
            CloseDB();
        }

        int init(std::string db_directory);

        int OpenDB(std::string db_directory);

        int CloseDB() {
            if (db != nullptr) {
                delete db;
                db = nullptr;
            }

            return 0;
        }

        int CalculateTotalRecords();

        int ParseLevelFile(std::string& file_name);

        int ParseLevelName(std::string& file_name) {
            std::ifstream file;
            file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            try {
                file.open(file_name);
            }
            catch (const std::exception& e) {
                log::error("Failed to read levelname - Error: {}", e.what());

                return -1;
            }

            std::stringstream iss;
            iss << file.rdbuf();
            log::info("levelname.txt: Level name is '{}'", iss.str());
            file.close();

            return 0;
        }

        int ParseDB();

        int ParseDBKey(int x, int z);

        void ParseChunkKey(std::string_view key, const char* data, size_t size);
    private:
        leveldb::DB* db;
        std::unique_ptr<leveldb::Options> db_options;
        int total_record_count;
    };

    extern std::unique_ptr<MinecraftWorldLevelDB> world;

} // namespace smokey_bedrock_parser