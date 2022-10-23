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
    return test;
};

void readPayLoad(std::vector<char> payload, int location) {
    if (payload[location] == 0x0a) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        readPayLoad(payload, location + 3 + a);
    }
    if (payload[location] == 0x08) {
        int a = (int)payload[location + 2];
        std::cout << readTagName(payload, location, a);
        int b = (int)payload[location + 4 + a];
        std::cout << readTagName(payload, location + 6, b);
        readPayLoad(payload, location + 5 + a + b);
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