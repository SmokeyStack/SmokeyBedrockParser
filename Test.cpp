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
    char nbt_bytes[2048]{};
    int string_size{-1};
    int count{0};
    int key_count{0};
    std::string root_name{};
    std::array key_name = {"name", "value"};

    for (it = ReadBytes(argv[1]).begin(); it != ReadBytes(argv[1]).end();
         it++) {
        nbt_bytes[count] = *it;
        count++;
    }

    for (int a = 0; a < length; a++) printf("%02x", nbt_bytes[a]);

    std::cout << "\t|\tOriginal Bytes\n---\n";

    if (ReadBytes(argv[1])[0] == 0x0a) {
        std::string value{};
        char* pc = (char*)std::memchr(nbt_bytes, 0x00, sizeof nbt_bytes);

        while ((pc - nbt_bytes + 3 + string_size) < count) {
            value = "";
            for (int a = 0; a < length; a++) printf("%02x", nbt_bytes[a]);

            pc = (char*)std::memchr(nbt_bytes, 0x00, sizeof nbt_bytes);
            string_size = ReadBytes(argv[1])[pc - nbt_bytes + 1];

            if (pc != nullptr)
                printf("\t|\tFound at: %d\t|\tString Size: %d\n",
                       pc - nbt_bytes + 1, string_size);

            if ((pc - nbt_bytes + 1) == 2) {
                for (size_t a = pc - nbt_bytes + 2;
                     a < pc - nbt_bytes + 2 + string_size; a++)
                    root_name += ReadBytes(argv[1])[a];

                j["name"] = root_name;
                std::string tag{getTag(ReadBytes(argv[1]),
                                       pc - nbt_bytes + 2 + string_size)};
                j["value"] = {{{"tag_type", tag}}};
            } else {
                for (size_t a = pc - nbt_bytes + 2;
                     a < pc - nbt_bytes + 2 + string_size; a++)
                    value += ReadBytes(argv[1])[a];
                j["value"][0].push_back({key_name[key_count], value});
                key_count++;
            }

            for (size_t a = 0; a < pc - nbt_bytes + string_size; a++)
                nbt_bytes[a] = '\b';
        }
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