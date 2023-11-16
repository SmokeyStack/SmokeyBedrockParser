#pragma once

#include <string>

namespace smokey_bedrock_parser {
    class Block {
    public:
        std::string name;
        std::tuple<int, int, int> color;

        Block(const Block&) = delete;
        void operator=(const Block&) = delete;

        static Block& GetInstance(const std::string& name, std::tuple<int, int, int> color);
        static Block* Add(const std::string& name, std::tuple<int, int, int> color);
        static Block* Get(const std::string& name);
    private:
        Block(const std::string& name, std::tuple<int, int, int> color) {
            this->name = name;
            this->color = color;
        }
    };
}