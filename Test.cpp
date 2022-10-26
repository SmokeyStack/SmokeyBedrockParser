#include <cstring>
#include <fstream>
#include <iostream>
#include <typeinfo>

#include "json.hpp"

using json = nlohmann::json;
json j;

static std::vector<char> ReadBytes(char const* filename) {
    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
    std::ifstream::pos_type pos = ifs.tellg();
    std::vector<char> result(pos);
    ifs.seekg(0, std::ios::beg);
    ifs.read(&result[0], pos);

    return result;
}

std::string readTagName(std::vector<char> payload, int location, size_t size) {
    std::string test{};
    for (auto it = 0; it < size; it++) test += payload[location + 3 + it];
    printf("%d - ", size);
    return test;
}

float readGloat(std::vector<char> payload, int location) {
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

int8_t readByte(std::vector<char> payload, int location) {
    union {
        char c[1];
        int8_t b;
    } u;

    u.c[0] = payload[location];

    return u.b;
}

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

double readDouble(std::vector<char> payload, int location) {
    union {
        char c[8];
        double f;
    } u;

    int count{0};
    for (int a = 7; a > 0; a--) {
        u.c[a] = payload[location + count];
        count++;
    }

    return u.f;
}

int16_t readShort(std::vector<char> payload, int location) {
    union {
        char c[2];
        int16_t s;
    } u;

    u.c[1] = payload[location];
    u.c[0] = payload[location + 1];

    return u.s;
}

void readPayLoad(std::vector<char> payload, int location) {
    // printf("\n-----\n");
    printf("\n-----\n 0x%02x - ", payload[location]);
    if (payload[location] == 0x00 && location <= payload.size()) {
        // std::cout << location + 1;
        readPayLoad(payload, location + 1);
    }
    if (payload[location] == 0x0a) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        readPayLoad(payload, location + 3 + a);
    }
    if (payload[location] == 0x08) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        int b = (int)payload[location + 4 + a];
        std::cout << ": " << readTagName(payload, location + a + 2, b);
        readPayLoad(payload, location + 5 + a + b);
    }
    if (payload[location] == 0x05) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        printf(": %f", readGloat(payload, location + 3 + a));
        readPayLoad(payload, location + 7 + a);
    }
    if (payload[location] == 0x03) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        printf(": %d", readInt(payload, location + 3 + a));
        readPayLoad(payload, location + 7 + a);
    }
    if (payload[location] == 0x01) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        printf(": %d", readByte(payload, location + 3 + a));
        readPayLoad(payload, location + 4 + a);
    }
    if (payload[location] == 0x04) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        std::cout << ": " << readLong(payload, location + 3 + a);
        readPayLoad(payload, location + 11 + a);
    }
    if (payload[location] == 0x06) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        std::cout << ": " << readDouble(payload, location + 3 + a);
        readPayLoad(payload, location + 11 + a);
    }
    if (payload[location] == 0x02) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        std::cout << ": " << readShort(payload, location + 3 + a);
        readPayLoad(payload, location + 5 + a);
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
                           readGloat(payload, location + 8 + a + (count * 4)));
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

    // std::ifstream thingy(argv[1], std::ios::binary);
    // std::vector<char> bytes((std::istreambuf_iterator<char>(thingy)),
    //                         (std::istreambuf_iterator<char>()));
    // thingy.close();

    // for (iter = bytes.begin(); iter != bytes.end(); iter++)
    //     std::cout << *iter << " ";

    return 0;
}