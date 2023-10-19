#include "logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace smokey_bedrock_parser {
	auto CreateConsoleSink() {
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_pattern("[%^%7l%$] %v");
		console_sink->set_color(spdlog::level::info, 100);
		return console_sink;
	}

	void SetupLoggerStage1() {
		spdlog::set_default_logger(std::make_shared<spdlog::logger>("stage_1", CreateConsoleSink()));
	}

	void SetupLoggerStage2(const std::filesystem::path& outpath, Level consoleLevel, Level fileLevel) {
		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(outpath.generic_string());
		// file_sink always record all log
		file_sink->set_level(spdlog::level::level_enum(fileLevel));
		file_sink->set_pattern("[%Y-%m-%d %T.%e][%L] %v");
		auto console_sink = CreateConsoleSink();

		console_sink->set_level(spdlog::level::level_enum(consoleLevel));

		spdlog::sinks_init_list sink_list = { file_sink, console_sink };

		spdlog::set_default_logger(std::make_shared<spdlog::logger>("stage_2", sink_list));
		// set global log level
		// TODO we can't assume int(debug) < int(info) < int(warn)
		spdlog::set_level(spdlog::level::level_enum(consoleLevel > fileLevel ? fileLevel : consoleLevel));
	}
}