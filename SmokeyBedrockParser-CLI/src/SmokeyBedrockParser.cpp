#include <boost/program_options.hpp>
#include <iostream>
#include <SmokeyBedrockParser-Core/logger.h>
#include <SmokeyBedrockParser-Core/world/world.h>
#include <SmokeyBedrockParser-Core/json/json.hpp>

int main(int argc, char** argv) {
	if (argc < 2) {
		smokey_bedrock_parser::log::error("No world path provided.");
		return 1;
	}

	smokey_bedrock_parser::SetupLoggerStage1();

	auto console_log_level = smokey_bedrock_parser::Level::Info;
	auto file_log_level = smokey_bedrock_parser::Level::Trace;
	std::filesystem::path log_directory = "logs/debug.log";

	SetupLoggerStage2(log_directory, console_log_level, file_log_level);

	smokey_bedrock_parser::world = std::make_unique<smokey_bedrock_parser::MinecraftWorldLevelDB>();

	int result = smokey_bedrock_parser::world->init(argv[1]);

	if (result != 0) {
		smokey_bedrock_parser::log::error("Failed to open world: {}", result);
		return result;
	}

	boost::program_options::options_description description("Allowed options");

	description.add_options()
		("help", "Produce help message")
		("listkeys", "Print All Record Keys")
		("get", boost::program_options::value<std::string>(), "Get the Data of a Key")
		("put", boost::program_options::value<std::vector<std::string>>()->multitoken(), "Put the Data of a Key");

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), vm);
	boost::program_options::notify(vm);

	if (vm.count("help")) {
		std::cout << description << "\n";
		return 1;
	}

	smokey_bedrock_parser::world->OpenDB(argv[1]);

	if (vm.count("listkeys")) {
		auto parse = smokey_bedrock_parser::world->ParseDB();

		for (const auto& [key, value] : parse) {
			smokey_bedrock_parser::log::info(key);
			smokey_bedrock_parser::log::info("---------");
			for (const auto& record : value)
				smokey_bedrock_parser::log::info(record);
			smokey_bedrock_parser::log::info("---------");
		}
	}

	if (vm.count("get")) {
		std::string key = vm["get"].as<std::string>();
		smokey_bedrock_parser::log::info(key);
		auto parse = smokey_bedrock_parser::world->GetKey(key);
		smokey_bedrock_parser::log::info(parse.dump(4, ' ', false, nlohmann::detail::error_handler_t::ignore));
	}

	if (vm.count("put")) {
		std::vector<std::string> key = vm["put"].as<std::vector<std::string>>();
		nlohmann::json json;
		std::ifstream file(key[1]);
		file >> json;
		auto parse = smokey_bedrock_parser::world->EditKey(key[0], json);
		//smokey_bedrock_parser::log::info(parse.dump(4, ' ', false, nlohmann::detail::error_handler_t::ignore));
	}

	return 0;
}