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

	world = std::make_unique<MinecraftWorldLevelDB>();

	world->init(argv[1]);
	world->dbOpen(argv[1]);
	world->dbParse();
	world->dbClose();

	log::info("Done.");

	return 0;
}