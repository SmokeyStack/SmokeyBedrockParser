#include <cstring>
#include <fstream>
#include <iostream>
#include <typeinfo>

#include "json.hpp"

using json = nlohmann::json;
json j;

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
std::string readTagName(std::vector<char> payload, int location, size_t size) {
    std::string test{};

    for (auto it = 0; it < size; it++) test += payload[location + 3 + it];

    printf("%d - ", size);

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
int8_t readByte(std::vector<char> payload, int location) {
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
 * @returns
 * Short value
 */
int16_t readShort(std::vector<char> payload, int location) {
    union {
        char c[2];
        int16_t s;
    } u;

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
 * @returns
 * Int value
 */
int32_t readInt(std::vector<char> payload, int location) {
    union {
        char c[4];
        int32_t i;
    } u;

    int count{0};

    for (int a = 3; a > 0; a--) {
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
 * @returns
 * Long value
 */
int64_t readLong(std::vector<char> payload, int location) {
    std::stringstream ss;
    std::string data;
    int count{0};

    for (int a = 0; a < 8; a++) {
        ss << std::hex << (int)payload[location + count];
        std::string test = ss.str();

        if (test.length() > 2)
            data += test.substr(test.length() - 2, 2);
        else
            data += test;

        count++;
    }

    std::istringstream converter{data};
    int64_t ll{0};
    converter >> std::hex >> ll;

    return ll;
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
 * @returns
 * Float value
 */
float readFloat(std::vector<char> payload, int location) {
    union {
        char c[4];
        float f;
    } u;

    int count{0};

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
 * @returns
 * Double value
 */
double readDouble(std::vector<char> payload, int location) {
    union {
        char c[8];
        double d;
    } u;

    int count{0};

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
void readPayLoad(std::vector<char> payload, int location) {
    printf("\n-----\n 0x%02x - ", payload[location]);

    if (payload[location] == 0x00 && location <= payload.size())
        readPayLoad(payload, location + 1);

    if (payload[location] == 0x01) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        printf(": %d", readByte(payload, location + 3 + a));
        readPayLoad(payload, location + 4 + a);
    }

    if (payload[location] == 0x02) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        std::cout << ": " << readShort(payload, location + 3 + a);
        readPayLoad(payload, location + 5 + a);
    }

    if (payload[location] == 0x03) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        printf(": %d", readInt(payload, location + 3 + a));
        readPayLoad(payload, location + 7 + a);
    }

    if (payload[location] == 0x04) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        std::cout << ": " << readLong(payload, location + 3 + a);
        readPayLoad(payload, location + 11 + a);
    }

    if (payload[location] == 0x05) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        printf(": %f", readFloat(payload, location + 3 + a));
        readPayLoad(payload, location + 7 + a);
    }

    if (payload[location] == 0x06) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        std::cout << ": " << readDouble(payload, location + 3 + a);
        readPayLoad(payload, location + 11 + a);
    }

    if (payload[location] == 0x08) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        int b = (int)payload[location + 4 + a];
        std::cout << ": " << readTagName(payload, location + a + 2, b);
        readPayLoad(payload, location + 5 + a + b);
    }

    if (payload[location] == 0x09) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a) << "\n";
        int count{0};
        int b{0};
        int loc{0};

        switch (payload[location + 3 + a]) {
            case 0x05:
                for (count; count < (int)payload[location + 7 + a]; count++)
                    printf("%f\n",
                           readFloat(payload, location + 8 + a + (count * 4)));

                readPayLoad(payload, location + 8 + a + (count * 4));

                break;
            case 0x08:
                for (count; count < (int)payload[location + 7 + a]; count++) {
                    std::cout << readTagName(payload, location + 7 + a + loc,
                                             payload[location + 9 + a + loc])
                              << "\n";
                    b = (int)payload[location + 9 + a + loc];
                    loc = loc + 2 + b;
                }

                readPayLoad(payload, location + 8 + a + loc);

                break;
            default:
                break;
        }
    }

    if (payload[location] == 0x0a) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        readPayLoad(payload, location + 3 + a);
    }
}

int main(int argc, char* argv[]) {
    int count{0};
    size_t length{ReadBytes(argv[1]).size()};
    std::vector<char> payload(ReadBytes(argv[1]).size());

    for (auto it = ReadBytes(argv[1]).begin(); it != ReadBytes(argv[1]).end();
         it++) {
        payload[count] = *it;
        count++;
    }

    for (int a = 0; a < length; a++) printf("%02x", payload[a]);

    std::cout << "\n-----\n";

    readPayLoad(payload, 0);

    // std::vector<char> test = {
    //     0x0a, 0x00, 0x00, 0x08, 0x04, 0x00, 0x6e, 0x61, 0x6d, 0x65, 0x0d,
    //     0x00, 0x6d, 0x69, 0x6e, 0x65, 0x63, 0x72, 0x61, 0x66, 0x74, 0x3a,
    //     0x61, 0x69, 0x72, 0x0a, 0x06, 0x00, 0x73, 0x74, 0x61, 0x74, 0x65,
    //     0x73, 0x00, 0x03, 0x07, 0x00, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f,
    //     0x6e, 0x01, 0x0a, 0x12, 0x01, 0x00, 0x0a, 0x00, 0x00, 0x08, 0x04,
    //     0x00, 0x6e, 0x61, 0x6d, 0x65, 0x0f, 0x00, 0x6d, 0x69, 0x6e, 0x65,
    //     0x63, 0x72, 0x61, 0x66, 0x74, 0x3a, 0x73, 0x74, 0x6f, 0x6e, 0x65,
    //     0x0a, 0x06, 0x00, 0x73, 0x74, 0x61, 0x74, 0x65, 0x73, 0x08, 0x0a,
    //     0x00, 0x73, 0x74, 0x6f, 0x6e, 0x65, 0x5f, 0x74, 0x79, 0x70, 0x65,
    //     0x05, 0x00, 0x73, 0x74, 0x6f, 0x6e, 0x65, 0x00, 0x03, 0x07, 0x00,
    //     0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x01, 0x0a, 0x12, 0x01,
    //     0x00};

    // readPayLoad(test, 0);

    return 0;
}

// 0x0a,0x00,0x00,0x08,0x04,0x00,0x6e,0x61,0x6d,0x65,0x0d,0x00,0x6d,0x69,0x6e,0x65,0x63,0x72,0x61,0x66,0x74,0x3a,0x61,0x69,0x72,0x0a,0x06,0x00,0x73,0x74,0x61,0x74,0x65,0x73,0x00,0x03,0x07,0x00,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x01,0x0a,0x12,0x01,0x00,0x0a,0x00,0x00,0x08,0x04,0x00,0x6e,0x61,0x6d,0x65,0x0f,0x00,0x6d,0x69,0x6e,0x65,0x63,0x72,0x61,0x66,0x74,0x3a,0x73,0x74,0x6f,0x6e,0x65,0x0a,0x06,0x00,0x73,0x74,0x61,0x74,0x65,0x73,0x08,0x0a,0x00,0x73,0x74,0x6f,0x6e,0x65,0x5f,0x74,0x79,0x70,0x65,0x05,0x00,0x73,0x74,0x6f,0x6e,0x65,0x00,0x03,0x07,0x00,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,0x01,0x0a,0x12,0x01,0x00

// 0a0000010004627974650002000573686f72740001030003696e7400000002050005666c6f617440400000060006646f75626c654010000000000000080006737472696e67000754657374696e670b0009696e745f6172726179000000030000002d000000050000000607000a627974655f6172726179000000030403020c000a6c6f6e675f6172726179000000030000000000000005000000000000000600000000000000070900046c69737403000000010000000409000c6175746f6c6973745f696e74030000000500000005000000060000000700007620fffffffffffffffffffffffeffffffac0900116175746f6c6973745f636f6d706f756e640a000000010800046e616d6500034142430600066865616c7468400c0000000000000000
// 0a0000010400627974650002050073686f72740100030300696e7402000000050500666c6f617400004040060600646f75626c650000000000001040080600737472696e67070054657374696e670b0900696e745f6172726179030000002d0000000500000006000000070a00627974655f6172726179030000000403020c0a006c6f6e675f6172726179030000000500000000000000060000000000000007000000000000000904006c697374030100000004000000090c006175746f6c6973745f696e74030500000005000000060000000700000020760000ffffffacfffffffeffffffffffffffff0911006175746f6c6973745f636f6d706f756e640a010000000804006e616d6503004142430606006865616c74680000000000000c400000