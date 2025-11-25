#include "logger.h"
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vector>

namespace gateway {
namespace core {

void Logger::init(const std::string &logPath, spdlog::level::level_enum level) {
  try {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(level);

    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        logPath, 1024 * 1024 * 5, 3); // 5MB size, 3 rotated files
    file_sink->set_level(level);

    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
    auto logger =
        std::make_shared<spdlog::logger>("gateway", sinks.begin(), sinks.end());

    logger->set_level(level);
    logger->flush_on(spdlog::level::warn);

    spdlog::set_default_logger(logger);
  } catch (const spdlog::spdlog_ex &ex) {
    // Fallback to simple console logging if init fails
    auto console = spdlog::stdout_color_mt("console");
    console->error("Log init failed: {}", ex.what());
  }
}

std::shared_ptr<spdlog::logger> Logger::get() { return spdlog::get("gateway"); }

} // namespace core
} // namespace gateway
