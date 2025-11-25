#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include <string>

namespace gateway {
namespace core {

class Logger {
public:
  static void init(const std::string &logPath = "logs/gateway.log",
                   spdlog::level::level_enum level = spdlog::level::info);

  static std::shared_ptr<spdlog::logger> get();
};

} // namespace core
} // namespace gateway

// Macros for easy logging
#define LOG_TRACE(...) gateway::core::Logger::get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) gateway::core::Logger::get()->debug(__VA_ARGS__)
#define LOG_INFO(...) gateway::core::Logger::get()->info(__VA_ARGS__)
#define LOG_WARN(...) gateway::core::Logger::get()->warn(__VA_ARGS__)
#define LOG_ERROR(...) gateway::core::Logger::get()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) gateway::core::Logger::get()->critical(__VA_ARGS__)
