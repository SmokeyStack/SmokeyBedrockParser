#pragma once

#include <filesystem>
#include <spdlog/spdlog.h>

namespace smokey_bedrock_parser {
    namespace log {
        using spdlog::trace;
        using spdlog::debug;
        using spdlog::info;
        using spdlog::warn;
        using spdlog::error;
    }

    enum class Level {
        Trace = spdlog::level::trace,
        Debug = spdlog::level::debug,
        Info = spdlog::level::info,
        Warn = spdlog::level::warn,
        Error = spdlog::level::err
    };

    void SetupLoggerStage1();

    void SetupLoggerStage2(const std::filesystem::path& outdir, Level consoleLevel, Level fileLevel);

} // namespace smokey_bedrock_parser