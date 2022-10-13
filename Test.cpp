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

std::string getTag(std::vector<char> test, int loc) {
    switch (test[loc]) {
        case 0x00:
            return "TAG_End";
        case 0x01:
            return "TAG_Byte";
        case 0x02:
            return "TAG_Short";
        case 0x03:
            return "TAG_Int";
        case 0x04:
            return "TAG_Long";
        case 0x05:
            return "TAG_Float";
        case 0x06:
            return "TAG_Double";
        case 0x07:
            return "TAG_Byte_Array";
        case 0x08:
            return "TAG_String";
        case 0x09:
            return "TAG_List";
        case 0x0A:
            return "TAG_Compound";
        case 0x0B:
            return "TAG_Int_Array";
        case 0x0C:
            return "TAG_Long_Array";
        default:
            return "Not a tag!";
    }
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

int main(int argc, char* argv[]) {
    std::vector<char>::iterator it;
    size_t length{ReadBytes(argv[1]).size()};
    char yoyo[2048]{};
    int count{0};

    std::string root_name{};

    if (ReadBytes(argv[1])[0] == 0x0a) {
        for (it = ReadBytes(argv[1]).begin(); it != ReadBytes(argv[1]).end();
             it++) {
            yoyo[count] = *it;
            count++;
        }

        int temp{};
        for (int a = 0; a < length; a++) printf("%02x", yoyo[a]);
        char* pc = (char*)std::memchr(yoyo, 0x00, sizeof yoyo);
        if (pc != nullptr) printf("\t|\tFound it at %d\n", pc - yoyo + 1);
        temp = ReadBytes(argv[1])[pc - yoyo + 1];
        for (size_t a = pc - yoyo + 2; a < pc - yoyo + 2 + temp; a++)
            root_name += ReadBytes(argv[1])[a];

        for (size_t a = 0; a < pc - yoyo + temp; a++) {
            yoyo[a] = '\b';
        }
        temp = 0;
        std::string name{};
        for (int a = 0; a < length; a++) printf("%02x", yoyo[a]);
        pc = (char*)std::memchr(yoyo, 0x00, sizeof yoyo);
        if (pc != nullptr) printf("\t|\tFound it at %d\n", pc - yoyo + 1);
        temp = ReadBytes(argv[1])[pc - yoyo + 1];
        for (size_t a = pc - yoyo + 2; a < pc - yoyo + 2 + temp; a++)
            name += ReadBytes(argv[1])[a];
        std::string tag{getTag(ReadBytes(argv[1]), pc - yoyo - 1)};

        for (size_t a = 0; a < pc - yoyo + temp; a++) {
            yoyo[a] = '\b';
        }
        temp = 0;
        std::string value{};
        for (int a = 0; a < length; a++) printf("%02x", yoyo[a]);
        pc = (char*)std::memchr(yoyo, 0x00, sizeof yoyo);
        if (pc != nullptr) printf("\t|\tFound it at %d\n", pc - yoyo + 1);
        temp = ReadBytes(argv[1])[pc - yoyo + 1];
        for (size_t a = pc - yoyo + 2; a < pc - yoyo + 2 + temp; a++)
            value += ReadBytes(argv[1])[a];

        j["name"] = root_name;
        // j["test"] = {{{"name", "val"}}};
        // j["test"][0].push_back({"value", "val"});
        j["value"] = {{{"tag_type", tag}, {"name", name}, {"value", value}}};
    }

    std::cout << j.dump(4);

    // std::ifstream thingy(argv[1], std::ios::binary);
    // std::vector<char> bytes((std::istreambuf_iterator<char>(thingy)),
    //                         (std::istreambuf_iterator<char>()));
    // thingy.close();

    // for (iter = bytes.begin(); iter != bytes.end(); iter++)
    //     std::cout << *iter << " ";

    return 0;
}