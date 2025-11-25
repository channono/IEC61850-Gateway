#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <vector>

namespace gateway {
namespace alarm {

enum class AlarmSeverity { INFO, WARNING, ERROR, CRITICAL };

struct Alarm {
  std::string id;
  std::string source; // IED Name / Node
  std::string message;
  AlarmSeverity severity;
  uint64_t timestamp;
  bool active;
  bool acknowledged;
};

class AlarmManager {
public:
  AlarmManager();
  ~AlarmManager();

  /**
   * @brief Raise a new alarm or update an existing one
   * @param source Source of the alarm
   * @param message Alarm message
   * @param severity Severity level
   */
  void raiseAlarm(const std::string &source, const std::string &message,
                  AlarmSeverity severity);

  /**
   * @brief Clear an active alarm
   * @param source Source of the alarm
   */
  void clearAlarm(const std::string &source);

  /**
   * @brief Acknowledge an alarm
   * @param alarmId ID of the alarm
   */
  void acknowledgeAlarm(const std::string &alarmId);

  /**
   * @brief Get all active alarms
   * @return Vector of active alarms
   */
  std::vector<Alarm> getActiveAlarms() const;

private:
  std::vector<Alarm> alarms_;
  mutable std::mutex mutex_;

  std::string generateId();
};

} // namespace alarm
} // namespace gateway
