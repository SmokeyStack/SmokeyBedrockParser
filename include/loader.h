#pragma once

#include <string>

#include "json.hpp"

namespace smokey_bedrock_parser {
    int32_t LoadBlocks(nlohmann::json json);
    int32_t LoadJson(const std::string& path);
} // namespace smokey_bedrock_parser