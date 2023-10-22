#include <cstdio>
#include <filesystem>
#include <string>
#include <cstdint>
#include <iostream>

#include <leveldb/db.h>
#include <leveldb/cache.h>


#include "world/world.h"

namespace smokey_bedrock_parser {}

int main(int argc, char** argv) {
	using namespace smokey_bedrock_parser;

	SetupLoggerStage1();
	
	auto console_log_level = Level::Info;
	auto file_log_level = Level::Trace;
	std::filesystem::path log_directory = "logs/debug.log";
	
	SetupLoggerStage2(log_directory, console_log_level, file_log_level);
	
	world = std::make_unique<MinecraftWorldLevelDB>();
	world->init(argv[1]);
	world->dbOpen(argv[1]);
	world->dbParse();
	world->dbClose();
	log::info("Done.");
	log::info("====================================================================================================");

	return 0;
}