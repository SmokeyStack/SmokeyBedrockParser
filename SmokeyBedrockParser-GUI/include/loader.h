#pragma once

#include <SmokeyBedrockParser-Core/json/json.hpp>
#include <string>

int LoadBlocks(nlohmann::json json);
int LoadJson(const std::string& path);