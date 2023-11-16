#pragma once

#include <climits>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "imgui/imgui.h"
#include "logger.h"
#include "minecraft/block.h"
#include "world/chunk.h"

namespace smokey_bedrock_parser {
    const std::vector<std::string> dimension_id_names{ "overworld","nether","the-end" };

    class Dimension {
    public:
        Dimension() {
            dimension_name = "(UNKNOWN)";
            dimension_id = -1;
            chunk_bounds_valid = false;
            min_chunk_x = INT32_MAX;
            max_chunk_x = INT32_MIN;
            min_chunk_z = INT32_MAX;
            max_chunk_z = INT32_MIN;
        }

        void set_dimension_name(std::string name) {
            dimension_name = name;
        }

        std::string get_dimension_name() {
            return dimension_name;
        }

        void set_dimension_id(int id) {
            dimension_id = id;
        }

        int get_dimension_id() {
            return dimension_id;
        }

        int get_min_chunk_x() {
            return min_chunk_x;
        }

        int get_max_chunk_x() {
            return max_chunk_x;
        }

        int get_min_chunk_z() {
            return min_chunk_z;
        }

        int get_max_chunk_z() {
            return max_chunk_z;
        }

        void UnsetChunkBoundsValid() {
            min_chunk_x = INT32_MAX;
            max_chunk_x = INT32_MIN;
            min_chunk_z = INT32_MAX;
            max_chunk_z = INT32_MIN;
            chunk_bounds_valid = false;
        }

        bool GetChunkBoundsValid() {
            return chunk_bounds_valid;
        }

        void SetChunkBoundsValid() {
            chunk_bounds_valid = true;
        }

        void ReportChunkBounds() {
            log::info("Bounds (chunk): Dimension Id={} X=({} {}) Z=({} {})",
                dimension_id, min_chunk_x, max_chunk_x, min_chunk_z, max_chunk_z);
        }

        void AddToChunkBounds(int chunk_x, int chunk_z) {
            min_chunk_x = std::min(min_chunk_x, chunk_x);
            max_chunk_x = std::max(max_chunk_x, chunk_x);
            min_chunk_z = std::min(min_chunk_z, chunk_z);
            max_chunk_z = std::max(max_chunk_z, chunk_z);
        }

        int AddChunk(int chunk_format_version, int chunk_x, int chunk_y, int chunk_z, const char* buffer,
            size_t buffer_length) {
            ChunkKey key(chunk_x, chunk_z);

            if (chunk_format_version == 7) {
                if (!chunks_has_key(chunks, key))
                    chunks[key] = std::unique_ptr<Chunk>(new Chunk());

                chunks[key]->ParseChunk(chunk_x, chunk_y, chunk_z, buffer, buffer_length, dimension_id, dimension_name);

                return 0;
            }
            else {
                log::error("Unknown chunk format version (version = {})", chunk_format_version);

                return 1;
            }
        };

        bool DoesChunkExist(int x, int z) {
            ChunkKey key(x, z);

            if (!chunks_has_key(chunks, key))
            {
                log::trace("Chunk does not exist (x = {}, z = {})", x, z);
                return false;
            }

            //log::info("Chunk does exist (x = {}, z = {})", x, z);

            return true;
        };

        void DrawChunk(int chunk_x, int chunk_z, ImDrawList* draw_list, ImVec2 origin, int grid_step) {
            if (DoesChunkExist(chunk_x, chunk_z)) {
                ChunkKey key(chunk_x, chunk_z);
                auto blocks = chunks[key]->blocks;
                int offset_chunk_x = chunk_x * grid_step;
                int offset_chunk_z = chunk_z * grid_step;
                grid_step /= 16;

                for (int x = 0; x < 16; x++)
                    for (int z = 0; z < 16; z++)
                        if ((Block::Get(blocks[x][z]) != nullptr)) {
                            int r = std::get<0>(Block::Get(blocks[x][z])->color);
                            int g = std::get<1>(Block::Get(blocks[x][z])->color);
                            int b = std::get<2>(Block::Get(blocks[x][z])->color);

                            draw_list->AddRectFilled(
                                ImVec2((origin[0] + grid_step * x) + offset_chunk_x, (origin[1] + grid_step * z) + offset_chunk_z),
                                ImVec2((origin[0] + grid_step * (x + 1)) + offset_chunk_x, (origin[1] + grid_step * (z + 1)) + offset_chunk_z),
                                IM_COL32(r, g, b, 255)
                            );
                        }
            }
        }
    private:
        std::string dimension_name;
        int dimension_id;
        typedef std::pair<int, int> ChunkKey;
        typedef std::map<ChunkKey, std::unique_ptr<Chunk>> ChunkMap;
        ChunkMap chunks;
        int min_chunk_x, max_chunk_x;
        int min_chunk_z, max_chunk_z;
        bool chunk_bounds_valid;

        bool chunks_has_key(const ChunkMap& m, const ChunkKey& k) {
            return m.find(k) != m.end();
        }
    };
} // namespace smokey_bedrock_parser