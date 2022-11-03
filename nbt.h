#ifndef NBT_H
#define NBT_H

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <vector>

#include "json.hpp"

using json = nlohmann::json;
json j = j["nbt"];

/**
 * Reads bytes from file
 *
 * @param filename
 * The location of the NBT file to read
 * @returns
 * Char vector of hex bytes
 */
static std::vector<char> ReadBytes(char const* filename) {
    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
    std::ifstream::pos_type pos = ifs.tellg();
    std::vector<char> result(pos);
    ifs.seekg(0, std::ios::beg);
    ifs.read(&result[0], pos);

    return result;
}

/**
 * Reads the name of NBT tag
 *
 * @param payload
 * The NBT bytes
 *
 * @param location
 * Location of the payload to start reading
 *
 * @param size
 * Length of the NBT tag name
 *
 * @returns
 * Name of NBT Tag
 */
static std::string readTagName(std::vector<char> payload, int location,
                               size_t size) {
    std::string test{};

    for (auto it = 0; it < size; it++) test += payload[location + 3 + it];

    printf("%zd - ", size);

    return test;
}

/**
 * Reads the byte tag
 *
 * @param payload
 * The NBT bytes
 *
 * @param location
 * Location of the payload to start reading
 *
 * @returns
 * Byte value
 */
static int8_t readByte(std::vector<char> payload, int location) {
    union {
        char c[1];
        int8_t b;
    } u;

    u.c[0] = payload[location];

    return u.b;
}

/**
 * Reads the short tag
 *
 * @param payload
 * The NBT bytes
 *
 * @param location
 * Location of the payload to start reading
 *
 * @param is_little
 * If true, the function reads in little-endian format
 * If false, the function reads in big-endian format
 *
 * @returns
 * Short value
 */
static int16_t readShort(std::vector<char> payload, int location,
                         bool is_little) {
    union {
        char c[2];
        int16_t s;
    } u;

    if (is_little) {
        u.c[0] = payload[location];
        u.c[1] = payload[location + 1];

        return u.s;
    }

    u.c[1] = payload[location];
    u.c[0] = payload[location + 1];

    return u.s;
}

/**
 * Reads the int tag
 *
 * @param payload
 * The NBT bytes
 *
 * @param location
 * Location of the payload to start reading
 *
 * @param is_little
 * If true, the function reads in little-endian format
 * If false, the function reads in big-endian format
 *
 * @returns
 * Int value
 */
static int32_t readInt(std::vector<char> payload, int location,
                       bool is_little) {
    union {
        char c[4];
        int32_t i;
    } u;

    int count{0};

    if (is_little) {
        for (int a = 0; a < 4; a++) {
            u.c[a] = payload[location + count];
            count++;
        }
        return u.i;
    }

    for (int a = 3; a > -1; a--) {
        u.c[a] = payload[location + count];
        count++;
    }

    return u.i;
}

/**
 * Reads the long tag
 *
 * @param payload
 * The NBT bytes
 *
 * @param location
 * Location of the payload to start reading
 *
 * @param is_little
 * If true, the function reads in little-endian format
 * If false, the function reads in big-endian format
 *
 * @returns
 * Long value
 */
static int64_t readLong(std::vector<char> payload, int location,
                        bool is_little) {
    union {
        char c[4];
        int64_t l;
    } u;

    if (is_little) {
        u.c[0] = payload[location];
        u.c[1] = payload[location + 1];
        u.c[2] = payload[location + 2];
        u.c[3] = payload[location + 3];
        u.c[4] = payload[location + 4];
        u.c[5] = payload[location + 5];
        u.c[6] = payload[location + 6];
        u.c[7] = payload[location + 7];

        return u.l;
    }

    u.c[7] = payload[location];
    u.c[6] = payload[location + 1];
    u.c[5] = payload[location + 2];
    u.c[4] = payload[location + 3];
    u.c[3] = payload[location + 4];
    u.c[2] = payload[location + 5];
    u.c[1] = payload[location + 6];
    u.c[0] = payload[location + 7];

    return u.l;
}

/**
 * Reads the float tag
 *
 * @param payload
 * The NBT bytes
 *
 * @param location
 * Location of the payload to start reading
 *
 * @param is_little
 * If true, the function reads in little-endian format
 * If false, the function reads in big-endian format
 *
 * @returns
 * Float value
 */
static float readFloat(std::vector<char> payload, int location,
                       bool is_little) {
    union {
        char c[4];
        float f;
    } u;

    int count{0};

    if (is_little) {
        for (int a = 0; a < 4; a++) {
            u.c[a] = payload[location + count];
            count++;
        }
        return u.f;
    }

    for (int a = 3; a > 0; a--) {
        u.c[a] = payload[location + count];
        count++;
    }

    return u.f;
}

/**
 * Reads the double tag
 *
 * @param payload
 * The NBT bytes
 *
 * @param location
 * Location of the payload to start reading
 *
 * @param is_little
 * If true, the function reads in little-endian format
 * If false, the function reads in big-endian format
 *
 * @returns
 * Double value
 */
static double readDouble(std::vector<char> payload, int location,
                         bool is_little) {
    union {
        char c[8];
        double d;
    } u;

    int count{0};

    if (is_little) {
        for (int a = 0; a < 8; a++) {
            u.c[a] = payload[location + count];
            count++;
        }
        return u.d;
    }

    for (int a = 7; a > 0; a--) {
        u.c[a] = payload[location + count];
        count++;
    }

    return u.d;
}

/**
 * Reads the NBT payload
 *
 * @param payload
 * The NBT bytes
 *
 * @param location
 * Location of the payload to start reading
 */
static void readPayLoad(std::vector<char> payload, int location, int endian) {
    bool is_little = false;
    if (endian == 1) is_little = true;

    if (location > payload.size()) std::cout << "\n\n" << j.dump(4) << "\n\n";

    if (payload[location] == 0x00 && location <= payload.size())
        readPayLoad(payload, location + 1, endian);

    if (payload[location] == 0x01) {
        int a = (int)payload[location + endian];
        std::string key = readTagName(payload, location, a);
        int8_t value = readByte(payload, location + 3 + a);
        j["nbt"].push_back({{"TagType", 1}, {"Name", key}, {"Value", value}});

        readPayLoad(payload, location + 4 + a, endian);
    }

    if (payload[location] == 0x02) {
        int a = (int)payload[location + endian];
        std::string key = readTagName(payload, location, a);
        int16_t value = readShort(payload, location + 3 + a, is_little);
        j["nbt"].push_back({{"TagType", 2}, {"Name", key}, {"Value", value}});

        readPayLoad(payload, location + 5 + a, endian);
    }

    if (payload[location] == 0x03) {
        int a = (int)payload[location + endian];
        std::string key = readTagName(payload, location, a);
        int32_t value = readInt(payload, location + 3 + a, is_little);
        j["nbt"].push_back({{"TagType", 3}, {"Name", key}, {"Value", value}});

        readPayLoad(payload, location + 7 + a, endian);
    }

    if (payload[location] == 0x04) {
        int a = (int)payload[location + endian];
        std::string key = readTagName(payload, location, a);
        int64_t value = readLong(payload, location + 3 + a);
        j["nbt"].push_back({{"TagType", 4}, {"Name", key}, {"Value", value}});
        readPayLoad(payload, location + 11 + a, endian);
    }

    if (payload[location] == 0x05) {
        int a = (int)payload[location + endian];
        std::string key = readTagName(payload, location, a);
        float value = readFloat(payload, location + 3 + a, is_little);
        j["nbt"].push_back({{"TagType", 5}, {"Name", key}, {"Value", value}});

        readPayLoad(payload, location + 7 + a, endian);
    }

    if (payload[location] == 0x06) {
        int a = (int)payload[location + endian];
        std::string key = readTagName(payload, location, a);
        double value = readDouble(payload, location + 3 + a, is_little);
        j["nbt"].push_back({{"TagType", 6}, {"Name", key}, {"Value", value}});

        readPayLoad(payload, location + 11 + a, endian);
    }

    if (payload[location] == 0x08) {
        int a = (int)payload[location + endian];
        std::string key = readTagName(payload, location, a);
        int b = (int)payload[location + 2 + endian + a];
        std::string value = readTagName(payload, location + a + 2, b);
        j["nbt"].push_back({{"TagType", 8}, {"Name", key}, {"Value", value}});

        readPayLoad(payload, location + 5 + a + b, endian);
    }

    if (payload[location] == 0x09) {
        int a = (int)payload[location + endian];
        std::string key = readTagName(payload, location, a);
        int count{0};
        int b{0};
        int loc{0};
        std::vector<float> floatArr{};
        switch (payload[location + 3 + a]) {
            case 0x00:
                j["nbt"].push_back(
                    {{"TagType", 9},
                     {"Name", key},
                     {"Value", {{"TagListType", 0}, {"List", nullptr}}}});

                readPayLoad(payload, location + 4 + a, endian);

                break;
            case 0x05:
                for (count; count < (int)payload[location + 4 + a]; count++)
                    floatArr.push_back(readFloat(
                        payload, location + 8 + a + (count * 4), is_little));

                j["nbt"].push_back(
                    {{"TagType", 9},
                     {"Name", key},
                     {"Value", {{"TagListType", 5}, {"List", floatArr}}}});

                readPayLoad(payload, location + 8 + a + (count * 4), endian);

                break;
            case 0x08:
                if (is_little) {
                    for (count; count < (int)payload[location + 4 + a];
                         count++) {
                        std::cout
                            << readTagName(payload, location + 7 + a + loc,
                                           payload[location + 8 + a + loc])
                            << "\n";
                        b = (int)payload[location + 8 + a + loc];
                        loc = loc + 2 + b;
                    }

                    readPayLoad(payload, location + 8 + a + loc, endian);

                    break;
                }

                for (count; count < (int)payload[location + 7 + a]; count++) {
                    std::cout << readTagName(payload, location + 7 + a + loc,
                                             payload[location + 9 + a + loc])
                              << "\n";
                    b = (int)payload[location + 9 + a + loc];
                    loc = loc + 2 + b;
                }

                readPayLoad(payload, location + 8 + a + loc, endian);

                break;
            case 0x0a:
                readPayLoad(payload, location + 8 + a, endian);

                break;
            default:
                break;
        }
    }

    if (payload[location] == 0x0a) {
        int a = (int)payload[location + endian];
        std::cout << readTagName(payload, location, a);
        readPayLoad(payload, location + 3 + a, endian);
    }
}

#endif