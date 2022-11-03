#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>

#include "leveldb/cache.h"
#include "leveldb/db.h"
#include "leveldb/decompress_allocator.h"
#include "leveldb/env.h"
#include "leveldb/filter_policy.h"
#include "leveldb/zlib_compressor.h"
#include "nbt.h"

std::string slice_to_hex_string(leveldb::Slice slice) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (size_t a = 0; a < slice.size(); a++)
        ss << std::setw(2)
           << static_cast<unsigned int>(static_cast<unsigned char>(slice[a]));

    return ss.str();
}

std::string hex_to_string(const std::string& s) {
    std::string output;
    size_t count = s.length() / 2;

    for (size_t a = 0; a < count; a++) {
        uint32_t b = 0;
        std::stringstream ss;
        ss << std::hex << s.substr(a * 2, 2);
        ss >> b;
        output.push_back(static_cast<unsigned char>(b));
    }

    return output;
}

std::string percent_encode(std::string_view str) {
    auto is_not_graph = [](unsigned char c) {
        return std::isgraph(c) == 0 || c == '%' || c == '@';
    };
    auto it = std::find_if(str.begin(), str.end(), is_not_graph);

    if (it == str.end()) return std::string{str};

    char buffer[8];
    std::string output;
    output.reserve(str.size());
    auto bit = str.begin();

    do {
        unsigned char c = *it;
        std::snprintf(buffer, 8, "%%%02hhX", c);
        output.append(bit, it);
        output.append(buffer);
        bit = ++it;
        it = std::find_if(it, str.end(), is_not_graph);
    } while (it != str.end());

    output.append(bit, str.end());

    return output;
}

bool is_chunk_key(std::string_view key) {
    auto tag_test = [](char tag) {
        return ((33 <= tag && tag <= 64) || tag == 118);
    };

    if (key.size() == 9 || key.size() == 10)
        return tag_test(key[8]);
    else if (key.size() == 13 || key.size() == 14)
        return tag_test(key[12]);

    return false;
}

char parse_chunk_key(std::string_view key) {
    char subtag{};
    char tag{};
    if (key.size() == 9) {
        tag = key[8];
        subtag = -1;
    } else if (key.size() == 10) {
        tag = key[8];
        subtag = key[9];
    } else if (key.size() == 13) {
        tag = key[12];
        subtag = -1;
    } else if (key.size() == 14) {
        tag = key[12];
        subtag = key[13];
    }
    return tag;
}
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <minecraft_world_dir> > list.tsv\n", argv[0]);
        return EXIT_FAILURE;
    }

    class NullLogger : public leveldb::Logger {
       public:
        void Logv(const char*, va_list) override {}
    };

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

    std::string path = std::string(argv[1]) + "/db";

    leveldb::Status status;
    leveldb::DB* pdb = nullptr;
    status = leveldb::DB::Open(options, path.c_str(), &pdb);
    auto db = std::unique_ptr<leveldb::DB>(pdb);

    if (!status.ok()) {
        fprintf(stderr, "ERROR: Opening '%s' failed.\n", path.c_str());
        return EXIT_FAILURE;
    }

    readOptions.verify_checksums = true;
    auto it = std::unique_ptr<leveldb::Iterator>{db->NewIterator(readOptions)};

    int how_many_keys = 0;
    int non_chunk_keys = 0;
    // 000000000000000031 - BE | 0000000000000000[0-9a-z]\\w+ - Chunk
    std::regex target("0000000000000000[0-9a-z]\\w+");

    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        leveldb::Slice key = it->key();
        size_t key_size = key.size();
        const char* key_data = key.data();
        leveldb::Slice value = it->value();

        if (strncmp(key_data, "BiomeData", key_size) == 0)
            std::cout << "Biome Data\n";

        else if (strncmp(key_data, "Overworld", key_size) == 0)
            std::cout << "Overworld\n";

        else if (strncmp(key_data, "~local_player", key_size) == 0)
            std::cout << "Local Player\n";

        else if ((key_size >= 7) && (strncmp(key_data, "player_", 7) == 0)) {
            std::string remote_player_id = std::string(
                &key_data[strlen("player_")], key_size - strlen("player_"));
            std::cout << "Remote Player - id: " << remote_player_id << "\n";
        } else if (strncmp(key_data, "game_flatworldlayers", key_size) == 0)
            std::cout
                << "game_flatworldlayers\n";  // MC Wiki says it's for flat
                                              // worlds, priority: low
        else if (strncmp(key_data, "VILLAGE_", 8) == 0)
            std::cout << "Village\n";

        else if (strncmp(key_data, "AutonomousEntities", key_size) == 0)
            std::cout << "AutonomousEntities\n";  // ??? MC Wiki help please?

        else if (strncmp(key_data, "LevelChunkMetaDataDictionary", key_size) ==
                 0)
            std::cout << "LevelChunkMetaDataDictionary\n";  // ??? MC Wiki help
                                                            // please?

        else if (strncmp(key_data, "actorprefix", 11) == 0)
            std::cout << "Entity found\n";

        else if (strncmp(key_data, "digp", 4) == 0)
            std::cout << "Pointer to entity\n";

        else if (strncmp(key_data, "mobevents", key_size) == 0)
            std::cout << "mobevents\n";

        else if (strncmp(key_data, "schedulerWT", key_size) == 0)
            std::cout << "schedulerWT\n";  // ??? MC Wiki help please?

        else if (strncmp(key_data, "scoreboard", key_size) == 0)
            std::cout << "scoreboard\n";

        else if (is_chunk_key({key_data, key_size}))
            std::cout << "Chunk\n";

        else {
            printf("\nUnknown record:\n-----\nkey_size: %d\t",
                   (int32_t)key.size());
            std::cout << percent_encode(hex_to_string(slice_to_hex_string(key)))
                      << "\n";
        }

        // if (is_chunk_key({key_data, key_size})) {
        //     // printf("0x%02x\t", parse_chunk_key({k.data(), k.size()}));
        //     // std::cout << slice_to_hex_string(k) << "\n-----\n";
        //     // std::cout << slice_to_hex_string(v) << "\n\n";
        //     // std::string ss = hex_to_string(slice_to_hex_string(v));
        //     // std::vector<char> test(ss.begin(), ss.end());
        //     // for (const char& c : test) printf("");
        //     // parseNBT(test, 0, 1);

        // } else {
        //     switch (key[8]) {
        //         case 102:
        //             // db->Delete(leveldb::WriteOptions(), k);  // Entity
        //             break;
        //         default:
        //             break;
        //     }

        //     std::cout << "Not a chunk!\t"
        //               << hex_to_string(slice_to_hex_string(key)) << "\n";
        //     // std::cout << k.data();
        //     non_chunk_keys++;
        //     // if (slice_to_hex_string(key) == "7e6c6f63616c5f706c61796572")
        //     // {
        //     //     std::string ss =
        //     //     hex_to_string(slice_to_hex_string(value));
        //     //     std::vector<char> test(ss.begin(), ss.end());
        //     //     for (const char& c : test) printf("0x%02x\n", c);
        //     //     parseNBT(test, 0, 1);
        //     // }
        //     // if (slice_to_hex_string(k) == "42696f6d6544617461") {
        //     //     std::string ss = hex_to_string(slice_to_hex_string(v));
        //     //     std::vector<char> test(ss.begin(), ss.end());
        //     //     for (const char& c : test) printf("");
        //     //     // parseNBT(test, 0, 1);
        //     // }
        // }

        how_many_keys++;
    }

    printf("Keys: %d\tNon Chunk Keys: %d\n", how_many_keys, non_chunk_keys);
    std::cout << slice_to_hex_string("Hello World") << "\n";
    std::cout << hex_to_string(slice_to_hex_string("Hello World"));

    if (!status.ok()) {
        fprintf(stderr, "ERROR: Reading '%s' failed: %s\n", path.c_str(),
                status.ToString().c_str());
        return EXIT_FAILURE;
    }

    delete &it;
    return EXIT_SUCCESS;
}